#pragma once
// Book for strict-FIFO markets with daily price limits and dense, increasing
// order ids (e.g. CN exchanges' per-channel sequence numbers).
//
// Hot path              data structure                          cost
// add (append back)     intrusive list per level, nodes by id   O(1), no alloc after warm-up
// execute / cancel      nodes_[id] -> unlink                    O(1), no hashing
// best price            flat array of levels indexed by tick    O(1); rescan only when best empties
//
// Hand-writes every verb its market uses. Has no modify and no primitives, so
// modify(book, msg) does not compile, and no generic algorithm applies.

#include <xgr/book/core.hpp>

#include <algorithm>
#include <limits>
#include <vector>

namespace __xgr::inline __v0::XGR_BOOK_NS {

class fifo_array_book {
  public:
    /// price range [lo, hi] in ticks of `tick` (e.g. today's limit-down .. limit-up)
    fifo_array_book(price_t lo, price_t hi, price_t tick)
        : lo_(lo), tick_(tick), n_(static_cast<std::uint32_t>((hi - lo) / tick + 1)),
          levels_{std::vector<level>(n_), std::vector<level>(n_)} {}

  private:
    static constexpr std::uint32_t nil = std::numeric_limits<std::uint32_t>::max();

    struct node {
        qty_t         qty  = 0;
        std::uint32_t prev = nil, next = nil, lvl = nil;
        side_t        side = side_t::bid;
        bool          live = false;
    };
    struct level {
        std::uint32_t head = nil, tail = nil, count = 0;
        qty_t         qty = 0;
    };

    price_t            lo_, tick_;
    std::uint32_t      n_;
    std::vector<level> levels_[2];          // [side][tick index]
    std::vector<node>  nodes_;              // index = order id
    std::uint32_t      best_[2] = {nil, nil};
    std::size_t        live_count_ = 0;

    static constexpr int si(side_t s) noexcept { return static_cast<int>(s); }
    static constexpr bool better(side_t s, std::uint32_t a, std::uint32_t b) noexcept {
        return s == side_t::bid ? a > b : a < b;
    }
    price_t price_at_(std::uint32_t i) const noexcept { return lo_ + static_cast<price_t>(i) * tick_; }

    std::uint32_t index_of_(price_t px) const noexcept {
        if (px < lo_ || (px - lo_) % tick_ != 0) return nil;
        auto const i = static_cast<std::uint64_t>((px - lo_) / tick_);
        return i < n_ ? static_cast<std::uint32_t>(i) : nil;
    }
    node *node_of_(order_id_t id) noexcept {
        return id < nodes_.size() && nodes_[id].live ? &nodes_[id] : nullptr;
    }
    node const *node_of_(order_id_t id) const noexcept {
        return id < nodes_.size() && nodes_[id].live ? &nodes_[id] : nullptr;
    }

    void link_back_(std::uint32_t id, side_t s, std::uint32_t li, qty_t q) {
        if (id >= nodes_.size()) nodes_.resize(std::max<std::size_t>(id + 1, nodes_.size() * 2));
        auto &l     = levels_[si(s)][li];
        nodes_[id]  = node{q, l.tail, nil, li, s, true};
        (l.tail != nil ? nodes_[l.tail].next : l.head) = id;
        l.tail = id;
        ++l.count;
        l.qty += q;
        ++live_count_;
        if (auto &b = best_[si(s)]; b == nil || better(s, li, b)) b = li;
    }

    void unlink_(std::uint32_t id) noexcept {
        auto &n = nodes_[id];
        auto &l = levels_[si(n.side)][n.lvl];
        (n.prev != nil ? nodes_[n.prev].next : l.head) = n.next;
        (n.next != nil ? nodes_[n.next].prev : l.tail) = n.prev;
        --l.count;
        l.qty -= n.qty;
        n.live = false;
        --live_count_;
        if (l.count == 0 && best_[si(n.side)] == n.lvl) rescan_(n.side, n.lvl);
    }

    void rescan_(side_t s, std::uint32_t from) noexcept { // walk toward worse prices
        auto const &ls = levels_[si(s)];
        auto       &b  = best_[si(s)];
        if (s == side_t::bid) {
            for (std::uint32_t i = from; i-- > 0;)
                if (ls[i].count) { b = i; return; }
        } else {
            for (std::uint32_t i = from + 1; i < n_; ++i)
                if (ls[i].count) { b = i; return; }
        }
        b = nil;
    }

    status reduce_(order_id_t id, qty_t by) noexcept {
        auto *n = node_of_(id);
        if (!n) return status::unknown_order;
        if (n->qty > by) {
            levels_[si(n->side)][n->lvl].qty -= by;
            n->qty -= by;
        } else {
            unlink_(static_cast<std::uint32_t>(id));
        }
        return status::ok;
    }

    // ------------------------------------------------ write side (hand-written)
    template <order_message M>
    friend status tag_invoke(insert_t, fifo_array_book &b, M const &m) {
        auto const id = get_id(m);
        auto const q  = get_qty(m);
        auto const li = b.index_of_(get_price(m));
        if (q <= 0 || li == nil || id >= nil) return status::rejected;
        if (b.node_of_(id)) return status::duplicate_order;
        b.link_back_(static_cast<std::uint32_t>(id), get_side(m), li, q);
        return status::ok;
    }

    template <class M> requires has_id<M>
    friend status tag_invoke(cancel_t, fifo_array_book &b, M const &m) noexcept {
        if constexpr (has_qty<M>) {
            return b.reduce_(get_id(m), get_qty(m));
        } else {
            if (!b.node_of_(get_id(m))) return status::unknown_order;
            b.unlink_(static_cast<std::uint32_t>(get_id(m)));
            return status::ok;
        }
    }

    template <class M> requires has_id<M> && has_qty<M>
    friend status tag_invoke(execute_t, fifo_array_book &b, M const &m) noexcept {
        return b.reduce_(get_id(m), get_qty(m));
    }

    template <class M>
    friend status tag_invoke(clear_t, fifo_array_book &b, M const &) noexcept {
        for (auto &ls : b.levels_) std::fill(ls.begin(), ls.end(), level{});
        b.nodes_.clear();
        b.best_[0] = b.best_[1] = nil;
        b.live_count_           = 0;
        return status::ok;
    }

    // ------------------------------------------------ read side
    friend std::optional<price_t> tag_invoke(best_price_t, fifo_array_book const &b, side_t s) noexcept {
        auto const i = b.best_[si(s)];
        return i == nil ? std::nullopt : std::optional{b.price_at_(i)};
    }
    friend std::optional<order_rec> tag_invoke(find_order_t, fifo_array_book const &b, order_id_t id) noexcept {
        auto const *n = b.node_of_(id);
        if (!n) return std::nullopt;
        return order_rec{id, n->side, b.price_at_(n->lvl), n->qty};
    }
    friend std::size_t tag_invoke(order_count_t, fifo_array_book const &b) noexcept { return b.live_count_; }

    template <class F>
    friend void tag_invoke(for_each_level_t, fifo_array_book const &b, side_t s, F &&f) {
        auto const  i  = b.best_[si(s)];
        auto const &ls = b.levels_[si(s)];
        if (i == nil) return;
        auto visit = [&](std::uint32_t j) {
            if (ls[j].count) f(level_view{b.price_at_(j), ls[j].qty, ls[j].count});
        };
        if (s == side_t::bid)
            for (std::uint32_t j = i + 1; j-- > 0;) visit(j);
        else
            for (std::uint32_t j = i; j < b.n_; ++j) visit(j);
    }
};

} // namespace __xgr::inline __v0::XGR_BOOK_NS

#pragma once
// Book for markets where the feed dictates queue position (ASX position,
// ASX24 priority): inserts land in the middle of a level.
//
// Hot path                  data structure                           cost
// insert at position / key  contiguous vector per level              memmove of a short level
// locate by id              hash id -> (side, price), scan in level  levels are short
// priority insert           append fast path, else upper_bound       usually O(1)
//
// Hand-writes nothing: implements the primitives + read side, and takes every
// verb from algo.hpp. Supports at_position and by_priority, but not at_back,
// so a message without position/priority is rejected at compile time.

#include <xgr/book/algo.hpp>

#include <algorithm>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

namespace __xgr::inline __v0::XGR_BOOK_NS {

template <class Rules>
class vector_level_book {
  public:
    using rules = Rules;

  private:
    struct entry {
        order_rec     rec;
        std::uint64_t key;
    };
    using level  = std::vector<entry>;
    using ladder = std::map<price_t, level>;
    using slot   = std::optional<typename level::iterator>;

    ladder                                                     sides_[2];
    std::unordered_map<order_id_t, std::pair<side_t, price_t>> index_;

    ladder       &lad_(side_t s) noexcept { return sides_[static_cast<int>(s)]; }
    ladder const &lad_(side_t s) const noexcept { return sides_[static_cast<int>(s)]; }

    template <class Self> static auto locate_(Self &self, order_id_t id) {
        using lad_it = decltype(self.lad_(side_t{}).begin());
        using ent_it = decltype(lad_it{}->second.begin());
        struct hit { lad_it lvl; ent_it it; };
        std::optional<hit> out;
        auto const f = self.index_.find(id);
        if (f == self.index_.end()) return out;
        auto lvl = self.lad_(f->second.first).find(f->second.second);
        auto it  = std::find_if(lvl->second.begin(), lvl->second.end(),
                                [&](entry const &e) { return e.rec.id == id; });
        out.emplace(hit{lvl, it});
        return out;
    }

    std::size_t ahead_(side_t s, price_t px) const noexcept { // orders at strictly better prices
        std::size_t n   = 0;
        auto const &lad = lad_(s);
        if (s == side_t::bid)
            for (auto it = lad.upper_bound(px); it != lad.end(); ++it) n += it->second.size();
        else
            for (auto it = lad.begin(); it != lad.lower_bound(px); ++it) n += it->second.size();
        return n;
    }

    template <class Pick> status put_(order_rec const &r, std::uint64_t key, Pick pick) {
        auto &lad         = lad_(r.side);
        auto [lvl, fresh] = lad.try_emplace(r.price);
        slot const where  = pick(lvl->second);
        if (!where) {
            if (fresh) lad.erase(lvl);
            return status::rejected;
        }
        lvl->second.insert(*where, entry{r, key});
        index_.emplace(r.id, std::pair{r.side, r.price});
        return status::ok;
    }

    // ------------------------------------------------ primitives
    friend status tag_invoke(prim::emplace_t, vector_level_book &b, order_rec const &r, at_position w) {
        std::size_t const skip = w.scope == position_scope::side ? b.ahead_(r.side, r.price) : 0;
        return b.put_(r, 0, [&](level &l) -> slot {
            if (w.pos == 0 || w.pos - 1 < skip || w.pos - 1 - skip > l.size()) return std::nullopt;
            return l.begin() + static_cast<std::ptrdiff_t>(w.pos - 1 - skip);
        });
    }

    friend status tag_invoke(prim::emplace_t, vector_level_book &b, order_rec const &r, by_priority w) {
        return b.put_(r, w.key, [&](level &l) -> slot {
            if (l.empty() || l.back().key <= w.key) return l.end(); // common case: newest last
            return std::upper_bound(l.begin(), l.end(), w.key,
                                    [](std::uint64_t k, entry const &e) { return k < e.key; });
        });
    }

    friend status tag_invoke(prim::erase_t, vector_level_book &b, order_id_t id) {
        auto h = locate_(b, id);
        if (!h) return status::unknown_order;
        auto &lad = b.lad_(h->it->rec.side);
        h->lvl->second.erase(h->it);
        if (h->lvl->second.empty()) lad.erase(h->lvl);
        b.index_.erase(id);
        return status::ok;
    }

    friend status tag_invoke(prim::set_qty_t, vector_level_book &b, order_id_t id, qty_t q) {
        auto h = locate_(b, id);
        if (!h) return status::unknown_order;
        h->it->rec.qty = q;
        return status::ok;
    }

    friend void tag_invoke(prim::wipe_t, vector_level_book &b) noexcept {
        b.sides_[0].clear();
        b.sides_[1].clear();
        b.index_.clear();
    }

    // ------------------------------------------------ read side
    friend std::optional<price_t> tag_invoke(best_price_t, vector_level_book const &b, side_t s) noexcept {
        auto const &lad = b.lad_(s);
        if (lad.empty()) return std::nullopt;
        return s == side_t::bid ? lad.rbegin()->first : lad.begin()->first;
    }
    friend std::optional<order_rec> tag_invoke(find_order_t, vector_level_book const &b, order_id_t id) {
        auto h = locate_(b, id);
        return h ? std::optional{h->it->rec} : std::nullopt;
    }
    friend std::size_t tag_invoke(order_count_t, vector_level_book const &b) noexcept { return b.index_.size(); }

    template <class F>
    friend void tag_invoke(for_each_level_t, vector_level_book const &b, side_t s, F &&f) {
        auto visit = [&](auto const &kv) {
            qty_t q = 0;
            for (auto const &e : kv.second) q += e.rec.qty;
            f(level_view{kv.first, q, static_cast<std::uint32_t>(kv.second.size())});
        };
        auto const &lad = b.lad_(s);
        if (s == side_t::bid) std::for_each(lad.rbegin(), lad.rend(), visit);
        else std::for_each(lad.begin(), lad.end(), visit);
    }

  public:
    /// debug view: order ids in queue order
    template <class F> void for_each_order(side_t s, price_t px, F &&f) const {
        if (auto it = lad_(s).find(px); it != lad_(s).end())
            for (auto const &e : it->second) f(e.rec);
    }
};

} // namespace __xgr::inline __v0::XGR_BOOK_NS

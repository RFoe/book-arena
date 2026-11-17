#pragma once
// OPTIONAL generic verbs. A book impl that exposes the four primitives below
// gets every verb it does not hand-write. Hand-written verbs always win
// (they are more specialized), so a book can hand-write only its hot paths.

#include "core.hpp"

namespace __xgr::inline __v0::XGR_BOOK_NS {

// ------------------------------------------------------------ placements
enum class position_scope : std::uint8_t { level, side };
struct at_back     {};
struct at_position { std::uint32_t pos; position_scope scope; }; // 1-based
struct by_priority { std::uint64_t key; };                       // smaller = earlier

// ------------------------------------------------------------ rules (knobs)
struct default_rules {
    static constexpr bool           allow_zero_qty        = false;
    static constexpr bool           qty_up_loses_priority = true;
    static constexpr position_scope position_kind         = position_scope::level;
};
template <class B> struct rules_of { using type = default_rules; };
template <class B> requires requires { typename B::rules; }
struct rules_of<B> { using type = typename B::rules; };
template <class B> using rules_of_t = typename rules_of<B>::type;

// ------------------------------------------------------------ primitives
namespace prim {
inline constexpr struct emplace_t : __facade<emplace_t> {} emplace; // (b, rec, placement) -> status
inline constexpr struct erase_t   : __facade<erase_t>   {} erase;   // (b, id)  -> status
inline constexpr struct set_qty_t : __facade<set_qty_t> {} set_qty; // (b, id, q) -> status, keeps position
inline constexpr struct wipe_t    : __facade<wipe_t>    {} wipe;    // (b)

template <class B>
concept store = readable_book<B> && requires(B &b, order_id_t id, qty_t q) {
    { erase(b, id) } -> std::same_as<status>;
    { set_qty(b, id, q) } -> std::same_as<status>;
};
template <class B, class Where>
concept can_emplace = requires(B &b, order_rec const &r, Where w) {
    { emplace(b, r, w) } -> std::same_as<status>;
};
} // namespace prim

// ------------------------------------------------------------ hook: placement(book, msg)
// Default from the message's fields; override per (book, msg) with tag_invoke.
inline constexpr struct placement_t : __facade<placement_t> {} placement;

template <class B, class M>
constexpr auto tag_invoke(placement_t, B const &, M const &m) noexcept {
    if constexpr (has_position<M>) return at_position{get_position(m), rules_of_t<B>::position_kind};
    else if constexpr (has_priority<M>) return by_priority{get_priority(m)};
    else return at_back{};
}

template <class B, class M>
concept __placeable =
    prim::can_emplace<B, std::invoke_result_t<placement_t const &, B const &, M const &>>;

template <class R>
constexpr bool __qty_ok(qty_t q) noexcept { return q > 0 || (q == 0 && R::allow_zero_qty); }

// ------------------------------------------------------------ generic verbs
template <prim::store B, order_message M>
    requires __placeable<B, M>
constexpr status tag_invoke(insert_t, B &b, M const &m) {
    order_rec const r{get_id(m), get_side(m), get_price(m), get_qty(m)};
    if (!__qty_ok<rules_of_t<B>>(r.qty)) return status::rejected;
    if (find_order(b, r.id)) return status::duplicate_order;
    return prim::emplace(b, r, placement(b, m));
}

template <prim::store B, class M>
    requires has_id<M>
constexpr status tag_invoke(cancel_t, B &b, M const &m) {
    auto const id = get_id(m);
    if constexpr (has_qty<M>) { // partial cancel by delta
        auto const cur = find_order(b, id);
        if (!cur) return status::unknown_order;
        qty_t const left = cur->qty - get_qty(m);
        return left > 0 ? prim::set_qty(b, id, left) : prim::erase(b, id);
    } else {
        return prim::erase(b, id);
    }
}

template <prim::store B, class M>
    requires has_id<M> && has_qty<M>
constexpr status tag_invoke(execute_t, B &b, M const &m) {
    auto const id  = get_id(m);
    auto const cur = find_order(b, id);
    if (!cur) return status::unknown_order;
    qty_t const left = cur->qty - get_qty(m);
    return left > 0 ? prim::set_qty(b, id, left) : prim::erase(b, id);
}

template <prim::store B, class M>
    requires has_id<M> && (has_qty<M> || has_price<M>) && __placeable<B, M>
constexpr status tag_invoke(modify_t, B &b, M const &m) {
    using R        = rules_of_t<B>;
    auto const cur = find_order(b, get_id(m));
    if (!cur) return status::unknown_order;

    order_rec next = *cur;
    if constexpr (has_qty<M>) next.qty = get_qty(m);
    if constexpr (has_price<M>) next.price = get_price(m);
    if (!__qty_ok<R>(next.qty)) return next.qty == 0 ? prim::erase(b, next.id) : status::rejected;

    bool const keep = !(has_position<M> || has_priority<M>)
                   && next.price == cur->price
                   && !(R::qty_up_loses_priority && next.qty > cur->qty);
    if (keep) return prim::set_qty(b, next.id, next.qty);
    if (auto s = prim::erase(b, next.id); s != status::ok) return s;
    return prim::emplace(b, next, placement(b, m));
}

template <class B, class M>
    requires requires(B &b) { prim::wipe(b); }
constexpr status tag_invoke(clear_t, B &b, M const &) {
    prim::wipe(b);
    return status::ok;
}

} // namespace __xgr::inline __v0::XGR_BOOK_NS

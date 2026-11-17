#pragma once
// Interfaces only, no implementation and no data structure.
//
//   write side  insert / cancel / modify / execute / clear, apply / decompose
//               -> each book impl implements the verbs its market needs, with
//                  the data structure that market's hot path needs.
//   read side   best_price / find_order / order_count / for_each_level
//               -> the unified interface strategies and snapshots consume.
//   messages    get_id / get_side / get_price / get_qty / get_position / get_priority

#include "facade.hpp"

#include <concepts>
#include <cstdint>
#include <optional>
#include <tuple>

namespace __xgr::inline __v0::XGR_BOOK_NS {

// ============================================================ vocabulary
enum class side_t : std::uint8_t { bid, ask };
using order_id_t = std::uint64_t;
using price_t    = std::int64_t;
using qty_t      = std::int64_t;

enum class status : std::uint8_t { ok, rejected, unknown_order, duplicate_order };

struct order_rec {
    order_id_t id;
    side_t     side;
    price_t    price;
    qty_t      qty;
};

struct level_view {
    price_t       price;
    qty_t         qty;    // sum of visible qty
    std::uint32_t orders;
};

// ============================================================ message queries
// Default: member of the conventional name. Foreign structs: tag_invoke.
inline constexpr struct get_id_t       : __facade<get_id_t>       {} get_id;
inline constexpr struct get_side_t     : __facade<get_side_t>     {} get_side;
inline constexpr struct get_price_t    : __facade<get_price_t>    {} get_price;
inline constexpr struct get_qty_t      : __facade<get_qty_t>      {} get_qty;
inline constexpr struct get_position_t : __facade<get_position_t> {} get_position;
inline constexpr struct get_priority_t : __facade<get_priority_t> {} get_priority;

template <class M> requires requires(M const &m) { m.id; }
constexpr order_id_t tag_invoke(get_id_t, M const &m) noexcept { return m.id; }
template <class M> requires requires(M const &m) { { m.side } -> std::convertible_to<side_t>; }
constexpr side_t tag_invoke(get_side_t, M const &m) noexcept { return m.side; }
template <class M> requires requires(M const &m) { m.price; }
constexpr price_t tag_invoke(get_price_t, M const &m) noexcept { return m.price; }
template <class M> requires requires(M const &m) { m.qty; }
constexpr qty_t tag_invoke(get_qty_t, M const &m) noexcept { return m.qty; }
template <class M> requires requires(M const &m) { m.position; }
constexpr std::uint32_t tag_invoke(get_position_t, M const &m) noexcept { return m.position; }
template <class M> requires requires(M const &m) { m.priority; }
constexpr std::uint64_t tag_invoke(get_priority_t, M const &m) noexcept { return m.priority; }

template <class Q, class M> concept __answers = std::invocable<Q const &, M const &>;
template <class M> concept has_id       = __answers<get_id_t, M>;
template <class M> concept has_side     = __answers<get_side_t, M>;
template <class M> concept has_price    = __answers<get_price_t, M>;
template <class M> concept has_qty      = __answers<get_qty_t, M>;
template <class M> concept has_position = __answers<get_position_t, M>;
template <class M> concept has_priority = __answers<get_priority_t, M>;

/// insert-able: carries everything an add needs
template <class M> concept order_message = has_id<M> && has_side<M> && has_price<M> && has_qty<M>;

// ============================================================ write side
inline constexpr struct insert_t  : __facade<insert_t>  {} insert;
inline constexpr struct cancel_t  : __facade<cancel_t>  {} cancel;
inline constexpr struct modify_t  : __facade<modify_t>  {} modify;
inline constexpr struct execute_t : __facade<execute_t> {} execute;
inline constexpr struct clear_t   : __facade<clear_t>   {} clear;

// message -> verb: `using verb_type = insert_t;` or specialize verb_of
template <class M> struct verb_of {};
template <class M> requires requires { typename M::verb_type; }
struct verb_of<M> { using type = typename M::verb_type; };
template <class M> using verb_of_t = typename verb_of<M>::type;
template <class M> concept atomic_message = requires { typename verb_of<M>::type; };

inline constexpr struct decompose_t : __facade<decompose_t> {} decompose; // (msg) -> tuple of msgs
inline constexpr struct apply_t     : __facade<apply_t>     {} apply;     // (book, msg) -> status

template <class B, class M>
    requires atomic_message<M> && std::invocable<verb_of_t<M> const &, B &, M const &>
constexpr status tag_invoke(apply_t, B &b, M const &m) {
    return __tag_v<verb_of_t<M>>(b, m);
}

template <class B, class M>
    requires (!atomic_message<M>) && std::invocable<decompose_t const &, M const &>
constexpr status tag_invoke(apply_t, B &b, M const &m) {
    return std::apply(
        [&b](auto const &...parts) {
            status s = status::ok;
            (void)(((s = apply(b, parts)) == status::ok) && ...);
            return s;
        },
        decompose(m));
}

/// Can book B consume message M? (checked at compile time)
template <class B, class M> concept book_for = std::invocable<apply_t const &, B &, M const &>;

// ============================================================ read side
inline constexpr struct best_price_t     : __facade<best_price_t>     {} best_price;     // (b, side) -> optional<price_t>
inline constexpr struct find_order_t     : __facade<find_order_t>     {} find_order;     // (b, id)   -> optional<order_rec>
inline constexpr struct order_count_t    : __facade<order_count_t>    {} order_count;    // (b)       -> size_t
inline constexpr struct for_each_level_t : __facade<for_each_level_t> {} for_each_level; // (b, side, f(level_view)), best -> worst

template <class B>
concept readable_book = requires(B const &b, side_t s, order_id_t id, void (*f)(level_view)) {
    { best_price(b, s) } -> std::same_as<std::optional<price_t>>;
    { find_order(b, id) } -> std::same_as<std::optional<order_rec>>;
    { order_count(b) } -> std::convertible_to<std::size_t>;
    for_each_level(b, s, f);
};

} // namespace __xgr::inline __v0::XGR_BOOK_NS

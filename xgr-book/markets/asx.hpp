#pragma once
#include <xgr/book/impl/vector_level_book.hpp>

namespace mkt::asx {
namespace bk = __xgr::book;

struct rules : bk::default_rules {
    static constexpr bool allow_zero_qty = true; // undisclosed orders
    // check your spec: per level or per side
    static constexpr bk::position_scope position_kind = bk::position_scope::level;
};
using book = bk::vector_level_book<rules>;

struct add_order      { using verb_type = bk::insert_t;  bk::order_id_t id; bk::side_t side; bk::price_t price; bk::qty_t qty; std::uint32_t position; };
struct modify_order   { using verb_type = bk::modify_t;  bk::order_id_t id; bk::price_t price; bk::qty_t qty; std::uint32_t position; };
struct delete_order   { using verb_type = bk::cancel_t;  bk::order_id_t id; };
struct order_executed { using verb_type = bk::execute_t; bk::order_id_t id; bk::qty_t qty; };
struct book_clear     { using verb_type = bk::clear_t; };
} // namespace mkt::asx

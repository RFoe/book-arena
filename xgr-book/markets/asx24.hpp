#pragma once
// Same hot path shape as ASX (mid-level inserts), so the same DS template is
// reused with different rules. A market with a different hot path brings its
// own book type instead.
#include "asx.hpp"

namespace mkt::asx24 {
namespace bk = __xgr::book;

struct rules : asx::rules {};
using book = bk::vector_level_book<rules>;

struct add_order    { using verb_type = bk::insert_t; bk::order_id_t id; bk::side_t side; bk::price_t price; bk::qty_t qty; std::uint64_t priority; };
struct modify_order { using verb_type = bk::modify_t; bk::order_id_t id; bk::price_t price; bk::qty_t qty; std::uint64_t priority; };
using asx::delete_order;
using asx::order_executed;
} // namespace mkt::asx24

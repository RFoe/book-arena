#pragma once
#include <xgr/book/impl/fifo_array_book.hpp>

namespace mkt::cn {
namespace bk = __xgr::book;

using book = bk::fifo_array_book;   // the DS this market's hot path needs

struct order  { using verb_type = bk::insert_t;  bk::order_id_t id; bk::side_t side; bk::price_t price; bk::qty_t qty; };
struct cancel { using verb_type = bk::cancel_t;  bk::order_id_t id; };
struct fill   { using verb_type = bk::execute_t; bk::order_id_t id; bk::qty_t qty; };
struct trade  { bk::order_id_t bid_id, ask_id; bk::price_t price; bk::qty_t qty; };   // composite

inline auto tag_invoke(bk::decompose_t, trade const &t) {
    return std::tuple{fill{t.bid_id, t.qty}, fill{t.ask_id, t.qty}};
}
} // namespace mkt::cn

#include <xgr/book/algo.hpp>
#include <cstdio>
namespace bk = __xgr::book;
struct mixed {   // primitives + read side, plus one hand-written hot verb
    int hand = 0, generic_emplace = 0;
    friend std::optional<bk::price_t> tag_invoke(bk::best_price_t, mixed const&, bk::side_t) { return {}; }
    friend std::optional<bk::order_rec> tag_invoke(bk::find_order_t, mixed const&, bk::order_id_t) { return {}; }
    friend std::size_t tag_invoke(bk::order_count_t, mixed const&) { return 0; }
    template <class F> friend void tag_invoke(bk::for_each_level_t, mixed const&, bk::side_t, F&&) {}
    friend bk::status tag_invoke(bk::prim::emplace_t, mixed& m, bk::order_rec const&, bk::at_back) { ++m.generic_emplace; return bk::status::ok; }
    friend bk::status tag_invoke(bk::prim::erase_t, mixed&, bk::order_id_t) { return bk::status::ok; }
    friend bk::status tag_invoke(bk::prim::set_qty_t, mixed&, bk::order_id_t, bk::qty_t) { return bk::status::ok; }
    template <bk::order_message M>
    friend bk::status tag_invoke(bk::insert_t, mixed& m, M const&) { ++m.hand; return bk::status::ok; }
};
struct add { bk::order_id_t id; bk::side_t side; bk::price_t price; bk::qty_t qty; };
struct del { bk::order_id_t id; };
int main() {
    mixed m;
    bk::insert(m, add{1, bk::side_t::bid, 1, 1});   // hand-written wins over generic
    bk::cancel(m, del{1});                          // generic (from primitives)
    std::printf("hand=%d generic_emplace=%d\n", m.hand, m.generic_emplace);
}

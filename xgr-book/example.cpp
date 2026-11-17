#include "markets/asx24.hpp"
#include "markets/cn.hpp"

#include <cstdio>

namespace bk = __xgr::book;
using bk::side_t;

constexpr char const *str(bk::status s) {
    constexpr char const *n[] = {"ok", "rejected", "unknown_order", "duplicate_order"};
    return n[static_cast<int>(s)];
}

// Consumer code: written once against the read-side concept, works on every book impl.
template <bk::readable_book B> void print(char const *title, B const &b) {
    auto bid = bk::best_price(b, side_t::bid), ask = bk::best_price(b, side_t::ask);
    std::printf("-- %s: %zu orders, best %lld / %lld\n", title, bk::order_count(b),
                bid ? static_cast<long long>(*bid) : -1, ask ? static_cast<long long>(*ask) : -1);
    for (side_t s : {side_t::ask, side_t::bid})
        bk::for_each_level(b, s, [&](bk::level_view l) {
            std::printf("   %s %5lld  qty %3lld  n %u\n", s == side_t::bid ? "bid" : "ask",
                        static_cast<long long>(l.price), static_cast<long long>(l.qty), l.orders);
        });
}

template <class B> void queue(B const &b, side_t s, bk::price_t px) {
    std::printf("   queue %s %lld:", s == side_t::bid ? "bid" : "ask", static_cast<long long>(px));
    b.for_each_order(s, px, [](bk::order_rec const &r) {
        std::printf(" #%llu(%lld)", static_cast<unsigned long long>(r.id), static_cast<long long>(r.qty));
    });
    std::printf("\n");
}

#define RUN(book, msg) std::printf("  %-44s -> %s\n", #msg, str(bk::apply(book, msg)))

int main() {
    {
        using namespace mkt::cn;
        book b{900, 1100, 1}; // limit-down .. limit-up
        std::puts("== cn  (fifo_array_book)");
        RUN(b, (order{1, side_t::bid, 1000, 10}));
        RUN(b, (order{2, side_t::bid, 999, 5}));
        RUN(b, (order{3, side_t::ask, 1001, 7}));
        RUN(b, (order{4, side_t::ask, 1200, 1}));  // outside price limits
        RUN(b, (trade{1, 3, 1001, 7}));            // composite; ask level empties
        RUN(b, (cancel{1}));                       // best bid empties -> rescan
        print("cn", b);
    }
    {
        using namespace mkt::asx;
        book b;
        std::puts("== asx  (vector_level_book<asx::rules>)");
        RUN(b, (add_order{10, side_t::bid, 100, 5, 1}));
        RUN(b, (add_order{11, side_t::bid, 100, 3, 2}));
        RUN(b, (add_order{12, side_t::bid, 100, 0, 1}));   // undisclosed, front
        RUN(b, (modify_order{11, 99, 8, 1}));
        RUN(b, (order_executed{10, 2}));
        print("asx", b);
        queue(b, side_t::bid, 100);
        RUN(b, (book_clear{}));
        print("asx after clear", b);
    }
    {
        using namespace mkt::asx24;
        book b;
        std::puts("== asx24  (vector_level_book<asx24::rules>)");
        RUN(b, (add_order{20, side_t::ask, 500, 1, 300}));
        RUN(b, (add_order{21, side_t::ask, 500, 2, 100}));
        RUN(b, (add_order{22, side_t::ask, 500, 3, 200}));
        RUN(b, (modify_order{21, 500, 2, 400}));
        RUN(b, (delete_order{20}));
        print("asx24", b);
        queue(b, side_t::ask, 500);
    }

    // What each book can consume is decided at compile time.
    static_assert(bk::readable_book<mkt::cn::book> && bk::readable_book<mkt::asx::book>);
    static_assert(!bk::book_for<mkt::cn::book, mkt::asx::modify_order>); // fifo_array_book has no modify
    static_assert(!bk::book_for<mkt::asx::book, mkt::cn::order>);        // no position -> at_back unsupported
    static_assert(bk::book_for<mkt::asx::book, mkt::asx::delete_order>); // generic cancel via primitives
}

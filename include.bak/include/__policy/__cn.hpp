#pragma once
#include <__xgr/__core.hpp>

namespace __policy::_CN_ {

using __price_t = __xgr::__price_t<std::ratio<100, 1>>;

struct __insert {
    using verb = __xgr::insert_t;
    __xgr::__id_t     id;
    __xgr::__side_t   side;
    __price_t         price;
    __xgr::__volume_t volume;
};

struct __cancel {
    using verb = __xgr::cancel_t;
    __xgr::__id_t id;
};

struct __execute {
    using verb = __xgr::execute_t;
    __xgr::__id_t     id;
    __xgr::__volume_t volume;
};

} // namespace __policy::_CN_

namespace __policy::_CN_::inline __v0 {

struct __book : __xgr::__crtp_book_base<__book> {
    // template <typename _M>
    // friend auto tag_invoke(__xgr::insert_t, __book &, _M const &__m) noexcept
    //     -> __xgr::_Rs {
    //     static_assert(
    //         __xgr::__has_id<_M> and __xgr::__has_side<_M> and
    //         __xgr::__has_price<_M> and __xgr::__has_volume<_M>);
    //     (void)__xgr::id(__m);
    //     (void)__xgr::side(__m);
    //     (void)__xgr::price(__m);
    //     (void)__xgr::volume(__m);
    //     return __xgr::_Rs::_S_ok;
    // }
    template <typename _M> auto insert(_M const &__m) const noexcept
        -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_side<_M> and
            __xgr::__has_price<_M> and __xgr::__has_volume<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::side(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::cancel_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(__xgr::__has_id<_M>);
        (void)__xgr::id(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::execute_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(__xgr::__has_id<_M> and __xgr::__has_volume<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::volume(__m);
        return __xgr::_Rs::_S_ok;
    }
};

} // namespace __policy::_CN_::inline __v0

namespace __policy::_CN_::inline __v1 {

struct __book : __xgr::__crtp_book_base<__book> {
    // template <typename _M>
    // friend auto tag_invoke(__xgr::insert_t, __book &, _M const &__m) noexcept
    //     -> __xgr::_Rs {
    //     static_assert(
    //         __xgr::__has_id<_M> and __xgr::__has_side<_M> and
    //         __xgr::__has_price<_M> and __xgr::__has_volume<_M>);
    //     (void)__xgr::id(__m);
    //     (void)__xgr::side(__m);
    //     (void)__xgr::price(__m);
    //     (void)__xgr::volume(__m);
    //     return __xgr::_Rs::_S_ok;
    // }
    template <typename _M> auto insert(_M const &__m) const noexcept
        -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_side<_M> and
            __xgr::__has_price<_M> and __xgr::__has_volume<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::side(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::cancel_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(__xgr::__has_id<_M>);
        (void)__xgr::id(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::execute_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(__xgr::__has_id<_M> and __xgr::__has_volume<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::volume(__m);
        return __xgr::_Rs::_S_ok;
    }
};

} // namespace __policy::_CN_::inline __v1

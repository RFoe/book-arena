#pragma once
// C++17 port of _BOOK_/include/__policy/__asx.hpp
#include <__xgr/__core.hpp>

namespace __xgr {
inline namespace __v0 {

using __position_t = __u32;

_XGR_FACADE(position);
template <typename _M>
constexpr auto tag_invoke(position_t, _M const &__m) noexcept
    -> decltype((void)__m.position, __position_t{}) {
    return __m.position;
}
template <typename _M>
inline constexpr bool __has_position = __answers<position_t, _M>;

} // namespace __v0
} // namespace __xgr

namespace __policy::_ASX_ {

using __price_t = __xgr::__price_t<std::ratio<10000, 1>>;

struct __insert {
    using verb = __xgr::insert_t;
    __xgr::__id_t       id;
    __xgr::__side_t     side;
    __xgr::__volume_t   volume;
    __xgr::__position_t position;
    __price_t           price;
};

struct __cancel {
    using verb = __xgr::cancel_t;
    __xgr::__id_t id;
};

struct __modify {
    using verb = __xgr::modify_t;
    __xgr::__id_t       id;
    __xgr::__position_t position;
    __xgr::__volume_t   volume;
    __price_t           price;
};

struct __execute {
    using verb = __xgr::execute_t;
    __xgr::__id_t     id;
    __xgr::__volume_t volume;
};

} // namespace __policy::_ASX_

namespace __policy::_ASX_ {
inline namespace __v0 {

struct __book : __xgr::__crtp_book_base<__book> {
    // template <typename _M>
    // friend auto tag_invoke(__xgr::insert_t, __book &, _M const &__m) noexcept
    //     -> __xgr::_Rs {
    //     static_assert(
    //         __xgr::__has_id<_M> and __xgr::__has_side<_M> and
    //         __xgr::__has_price<_M> and __xgr::__has_volume<_M> and
    //         __xgr::__has_position<_M>);
    //     (void)__xgr::id(__m);
    //     (void)__xgr::side(__m);
    //     (void)__xgr::price(__m);
    //     (void)__xgr::volume(__m);
    //     (void)__xgr::position(__m);
    //     return __xgr::_Rs::_S_ok;
    // }
    template <typename _M> auto insert(_M const &__m) noexcept -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_side<_M> and
            __xgr::__has_price<_M> and __xgr::__has_volume<_M> and
            __xgr::__has_position<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::side(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        (void)__xgr::position(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::modify_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_price<_M> and
            __xgr::__has_volume<_M> and __xgr::__has_position<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        (void)__xgr::position(__m);
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

} // namespace __v0
} // namespace __policy::_ASX_

namespace __policy::_ASX_ {
inline namespace __v1 {

struct __book : __xgr::__crtp_book_base<__book> {
    // template <typename _M>
    // friend auto tag_invoke(__xgr::insert_t, __book &, _M const &__m) noexcept
    //     -> __xgr::_Rs {
    //     static_assert(
    //         __xgr::__has_id<_M> and __xgr::__has_side<_M> and
    //         __xgr::__has_price<_M> and __xgr::__has_volume<_M> and
    //         __xgr::__has_position<_M>);
    //     (void)__xgr::id(__m);
    //     (void)__xgr::side(__m);
    //     (void)__xgr::price(__m);
    //     (void)__xgr::volume(__m);
    //     (void)__xgr::position(__m);
    //     return __xgr::_Rs::_S_ok;
    // }
    template <typename _M> auto insert(_M const &__m) noexcept -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_side<_M> and
            __xgr::__has_price<_M> and __xgr::__has_volume<_M> and
            __xgr::__has_position<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::side(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        (void)__xgr::position(__m);
        return __xgr::_Rs::_S_ok;
    }
    template <typename _M>
    friend auto tag_invoke(__xgr::modify_t, __book &, _M const &__m) noexcept
        -> __xgr::_Rs {
        static_assert(
            __xgr::__has_id<_M> and __xgr::__has_price<_M> and
            __xgr::__has_volume<_M> and __xgr::__has_position<_M>);
        (void)__xgr::id(__m);
        (void)__xgr::price(__m);
        (void)__xgr::volume(__m);
        (void)__xgr::position(__m);
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

} // namespace __v1
} // namespace __policy::_ASX_

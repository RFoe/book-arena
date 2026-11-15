#pragma once
#include <__concepts/convertible_to.h>
#include <__concepts/invocable.h>
#include <__xgr/__facade.hpp>
#include <asm-generic/types.h>
#include <ratio>

#include <__compare/strong_order.h> // IWYU pragma: export

namespace __xgr::inline __v0 {

template <typename _Tp> inline const bool __is_ratio_v = false;
template <std::intmax_t _Num, std::intmax_t _Den>
inline const bool __is_ratio_v<std::ratio<_Num, _Den>> = true;

enum class __side_t : __u8 { _S_bid, _S_ask };
enum class _Rs : __u8 { _S_ok, _S_reject, _S_bad, _S_dup };
using __id_t = __u64;

template <typename _R> struct [[_Clang::__no_specializations__]] __price_t {
    static_assert(__is_ratio_v<_R>);
    using __ratio [[__gnu__::__nodebug__]] = _R;
    std::intmax_t __val_;

    friend constexpr auto operator<=>(__price_t, __price_t) = default;
    friend constexpr auto operator+(__price_t a, __price_t b) noexcept
        -> __price_t {
        return {a.__val_ + b.__val_};
    }
    friend constexpr auto operator-(__price_t a, __price_t b) noexcept
        -> __price_t {
        return {a.__val_ - b.__val_};
    }
    constexpr auto to_double() const noexcept -> double {
        return double(__val_) * _R::num / _R::den;
    }
};

template <typename _Tp> inline const bool __is_price_v                = false;
template <typename _R> inline const bool  __is_price_v<__price_t<_R>> = true;
template <typename _Tp>
concept __price_like = __is_price_v<_Tp>;

template <class _To, class _S>
constexpr auto __price_cast(__price_t<_S> p) noexcept -> _To {
    using _R = std::ratio_divide<_S, typename _To::__ratio>;
    return {p.__val_ * _R::num / _R::den};
}
using __volume_t = __s64;

_XGR_FACADE(insert);
_XGR_FACADE(cancel);
_XGR_FACADE(modify);
_XGR_FACADE(execute);
_XGR_FACADE(clear);

_XGR_FACADE(id);
_XGR_FACADE(side);
_XGR_FACADE(price);
_XGR_FACADE(volume);

template <typename _M>
    requires requires(_M const &__m) { __m.id; }
constexpr auto tag_invoke(id_t, _M const &__m) noexcept -> __id_t {
    return __m.id;
}
template <typename _M>
    requires requires(_M const &__m) {
        { __m.side } -> std::convertible_to<__side_t>;
    }
constexpr auto tag_invoke(side_t, _M const &__m) noexcept -> __side_t {
    return __m.side;
}
template <typename _M>
constexpr auto tag_invoke(price_t, _M const &__m) noexcept -> __price_like
    auto {
    return __m.price;
}
template <typename _M>
    requires requires(_M const &__m) {
        { __m.volume } -> std::convertible_to<__volume_t>;
    }
constexpr auto tag_invoke(volume_t, _M const &__m) noexcept -> __volume_t {
    return __m.volume;
}

template <typename Q, typename _M>
concept __answers = std::invocable<Q const &, _M const &>;
template <typename _M>
concept __has_id = __answers<id_t, _M>;
template <typename _M>
concept __has_side = __answers<side_t, _M>;
template <typename _M>
concept __has_price = __answers<price_t, _M>;
template <typename _M>
concept __has_volume = __answers<volume_t, _M>;
/// insert-able: carries everything an add needs
template <typename _M>
concept __message =
    __has_id<_M> && __has_side<_M> && __has_price<_M> && __has_volume<_M>;

template <typename _M> struct __trait {};
template <typename _M>
    requires requires { typename _M::verb; }
struct __trait<_M> {
    using __verb = typename _M::verb;
};
template <typename _M> using __verb_t = typename __trait<_M>::__verb;
template <typename _M>
concept __atomic = requires { typename __trait<_M>::__verb; };

template <typename _Ty> inline constexpr _Ty __numb_;

template <typename _Derived> struct __crtp_book_base {
    template <__atomic _M> auto __apply(const _M &__m) noexcept
        -> std::convertible_to<_Rs> decltype(auto) {
        return __numb_<__verb_t<_M>>(static_cast<_Derived &>(*this), __m);
    }
    template <__atomic _M> auto __apply(const _M &__m) const noexcept
        -> std::convertible_to<_Rs> decltype(auto) {
        return __numb_<__verb_t<_M>>(static_cast<_Derived const &>(*this), __m);
    }
};

} // namespace __xgr::inline __v0

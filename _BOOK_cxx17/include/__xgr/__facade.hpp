#pragma once
// C++17 port of _BOOK_/include/__xgr/__facade.hpp
//
//   concepts            -> `inline constexpr bool` variable templates (same names)
//   requires-clauses    -> std::enable_if_t / partial specializations on void_t
//   deducing this       -> `operator() const` + static_cast to the derived tag
//   constrained partial specialization -> extra defaulted `void` parameter

#include <type_traits>
#include <utility>

#if defined(__clang__) && __has_cpp_attribute(_Clang::__no_specializations__)
    #define _XGR_NO_SPECIALIZATIONS [[_Clang::__no_specializations__]]
#else
    #define _XGR_NO_SPECIALIZATIONS
#endif

#if defined(__clang__)
    #define _XGR_NODEBUG [[__gnu__::__nodebug__]]
#else
    #define _XGR_NODEBUG
#endif

namespace __xgr {
inline namespace __v0 {

template <typename _T, typename _U>
inline constexpr bool __decays_to = std::is_same_v<std::decay_t<_T>, _U>;

template <typename _T>
inline constexpr bool __unqualified_class =
    std::is_class_v<_T> && std::is_same_v<_T, std::remove_cv_t<_T>>;

// std::is_scoped_enum_v is C++23
template <typename _T, bool = std::is_enum_v<_T>>
inline constexpr bool __scoped_enum = false;
template <typename _T>
inline constexpr bool __scoped_enum<_T, true> =
    !std::is_convertible_v<_T, std::underlying_type_t<_T>>;

template <typename _T, typename _X = std::decay_t<_T>>
inline constexpr bool __decay_enum_or_class =
    __scoped_enum<_X> or std::is_class_v<_X>;

// ------------------------------------------------------------ tag_invoke
template <typename _Void, typename _Tag, typename... _Args>
struct __tag_invocable_impl : std::false_type {};
template <typename _Tag, typename... _Args>
struct __tag_invocable_impl<
    std::void_t<decltype(tag_invoke(
        std::declval<_Tag>(), std::declval<_Args>()...))>,
    _Tag, _Args...> : std::true_type {};

template <typename _Tag, typename... _Args>
inline constexpr bool __tag_invocable =
    std::conjunction_v<
        std::bool_constant<__decay_enum_or_class<_Tag>>,
        __tag_invocable_impl<void, _Tag, _Args...>>;

template <typename _Tag, typename... _Args>
using tag_invoke_result_t _XGR_NODEBUG =
    decltype(tag_invoke(std::declval<_Tag>(), std::declval<_Args>()...));

template <bool, typename _Tag, typename... _Args>
struct __nothrow_tag_invocable_impl : std::false_type {};
template <typename _Tag, typename... _Args>
struct __nothrow_tag_invocable_impl<true, _Tag, _Args...>
    : std::bool_constant<noexcept(tag_invoke(
          std::declval<_Tag>(), std::declval<_Args>()...))> {};

template <typename _Tag, typename... _Args>
inline constexpr bool __nothrow_tag_invocable = __nothrow_tag_invocable_impl<
    __tag_invocable<_Tag, _Args...>, _Tag, _Args...>::value;

// ------------------------------------------------------------ member function
/// The tag forwards to a same-named member function (see _XGR_FACADE).
template <typename _Void, typename _Tag, typename... _Args>
struct __member_invocable_impl : std::false_type {};
template <typename _Tag, typename... _Args>
struct __member_invocable_impl<
    std::void_t<decltype(_Tag::__memb_invoke(std::declval<_Args>()...))>,
    _Tag, _Args...> : std::true_type {};

template <typename _Tag, typename... _Args>
inline constexpr bool __member_invocable =
    __member_invocable_impl<void, _Tag, _Args...>::value;

template <bool, typename _Tag, typename... _Args>
struct __nothrow_member_invocable_impl : std::false_type {};
template <typename _Tag, typename... _Args>
struct __nothrow_member_invocable_impl<true, _Tag, _Args...>
    : std::bool_constant<noexcept(
          _Tag::__memb_invoke(std::declval<_Args>()...))> {};

template <typename _Tag, typename... _Args>
inline constexpr bool __nothrow_member_invocable =
    __nothrow_member_invocable_impl<
        __member_invocable<_Tag, _Args...>, _Tag, _Args...>::value;

// ------------------------------------------------------------ __facade
template <typename T, typename = void>
// NOLINTNEXTLINE(*-member-functions)
struct _XGR_NO_SPECIALIZATIONS __facade final {
    __facade() noexcept                                     = delete;
    __facade(const __facade &) noexcept                     = delete;
    auto operator=(__facade const &) noexcept -> __facade & = delete;

    template <typename... Args> void operator()(Args &&...) = delete;
};

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunknown-warning-option"
    #pragma clang diagnostic ignored "-Winvalid-specialization"
#endif
template <typename _Tag>
// NOLINTNEXTLINE(*-member-functions)
struct __facade<_Tag, std::enable_if_t<__unqualified_class<_Tag>>> {
    // 1. obj.name(args...) if the first argument has that member function
    template <
        typename _Obj, typename... _Args,
        std::enable_if_t<__member_invocable<_Tag, _Obj, _Args...>, int> = 0>
    constexpr auto operator()(_Obj &&__obj, _Args &&...__args) const
        noexcept(__nothrow_member_invocable<_Tag, _Obj, _Args...>)
            -> decltype(auto) {
        return _Tag::__memb_invoke(
            static_cast<_Obj &&>(__obj), static_cast<_Args &&>(__args)...);
    }

    // 2. otherwise tag_invoke(tag, args...)
    template <
        typename... _Args,
        std::enable_if_t<
            not __member_invocable<_Tag, _Args...> and
                __tag_invocable<_Tag const &, _Args...>,
            int> = 0>
    constexpr auto operator()(_Args &&...__args) const
        noexcept(__nothrow_tag_invocable<_Tag const &, _Args...>)
            -> decltype(auto) {
        return tag_invoke(
            static_cast<_Tag const &>(*this), static_cast<_Args &&>(__args)...);
    }

  private:
    friend _Tag;

    constexpr __facade() noexcept                                     = default;
    constexpr __facade(const __facade &) noexcept                     = default;
    constexpr auto operator=(__facade const &) noexcept -> __facade & = default;
};
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

/// A usable object for any tag type (tags can't be brace-initialized; see
/// __facade).
template <class _Tag> inline constexpr _Tag __tag_v;

} // namespace __v0
} // namespace __xgr

#define _XGR_FACADE(_X)                                                        \
    inline constexpr struct _X##_t : __facade<_X##_t> {                        \
        template <typename _Obj, typename... _Args>                            \
        static constexpr auto __memb_invoke(_Obj &&__obj, _Args &&...__args)   \
            noexcept(noexcept(static_cast<_Obj &&>(__obj)._X(                  \
                static_cast<_Args &&>(__args)...)))                            \
                -> decltype(static_cast<_Obj &&>(__obj)._X(                    \
                    static_cast<_Args &&>(__args)...)) {                       \
            return static_cast<_Obj &&>(__obj)._X(                             \
                static_cast<_Args &&>(__args)...);                             \
        }                                                                      \
    } _X

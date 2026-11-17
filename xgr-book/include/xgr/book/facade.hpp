#pragma once

#include <type_traits>
#include <utility>

#ifndef XGR_BOOK_NS
#  define XGR_BOOK_NS book
#endif

#if __has_cpp_attribute(clang::no_specializations)
#  define XGR_NO_SPECIALIZATIONS [[clang::no_specializations]]
#else
#  define XGR_NO_SPECIALIZATIONS
#endif

namespace __xgr::inline __v0::XGR_BOOK_NS {

template <typename _T, typename _U>
concept __decays_to = std::is_same_v<std::decay_t<_T>, _U>;

template <typename _T>
concept __unqualified_class =
    std::is_class_v<_T> && std::is_same_v<_T, std::remove_cv_t<_T>>;

template <typename _T, typename _X = std::decay_t<_T>>
concept __decay_enum_or_class =
    std::is_scoped_enum_v<_X> or std::is_class_v<_X>;

template <typename _Tag, typename... _Args>
concept __tag_invocable =
    __decay_enum_or_class<_Tag> and requires(_Tag &&__tag, _Args &&...__args) {
        tag_invoke(
            static_cast<_Tag &&>(__tag), static_cast<_Args &&>(__args)...);
    };

template <typename _Tag, typename... _Args>
using tag_invoke_result_t [[__gnu__::__nodebug__]] =
    decltype(tag_invoke(std::declval<_Tag>(), std::declval<_Args>()...));

template <typename _Tag, typename... _Args>
concept __nothrow_tag_invocable =
    __tag_invocable<_Tag, _Args...> and
    noexcept(tag_invoke(std::declval<_Tag>(), std::declval<_Args>()...));

template <typename T>
// NOLINTNEXTLINE(*-member-functions)
struct XGR_NO_SPECIALIZATIONS __facade final {
    __facade() noexcept                                     = delete;
    __facade(const __facade &) noexcept                     = delete;
    auto operator=(__facade const &) noexcept -> __facade & = delete;

    template <typename... Args> void operator()(Args &&...) = delete;
};

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#  pragma clang diagnostic ignored "-Winvalid-specialization"
#endif
template <__unqualified_class _Tag>
// NOLINTNEXTLINE(*-member-functions)
struct __facade<_Tag> {
    template <__decays_to<_Tag> _U, typename... _Args>
        requires __tag_invocable<_U, _Args...>
    constexpr auto operator()(this _U &&__self, _Args &&...__args)
        noexcept(__nothrow_tag_invocable<_U, _Args...>) -> decltype(auto) {
        return tag_invoke(
            static_cast<_U &&>(__self), static_cast<_Args &&>(__args)...);
    }

  private:
    friend _Tag;

    constexpr __facade() noexcept                                     = default;
    constexpr __facade(const __facade &) noexcept                     = default;
    constexpr auto operator=(__facade const &) noexcept -> __facade & = default;
};
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

/// A usable object for any tag type (tags can't be brace-initialized; see __facade).
template <class _Tag> inline constexpr _Tag __tag_v;

} // namespace __xgr::inline __v0::XGR_BOOK_NS

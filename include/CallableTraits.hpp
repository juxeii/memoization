#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

namespace memoization::details
{
    template <typename T>
    struct callable_signature_impl
    {
    };

    template <typename R, typename... Args>
    struct callable_signature_impl<std::function<R(Args...)>>
    {
        using type = R(Args...);
    };

    template <typename T>
    struct callable_result_impl
    {
    };

    template <typename R, typename... Args>
    struct callable_result_impl<std::function<R(Args...)>>
    {
        using type = R;
    };

    template <typename T>
    struct callable_arguments_impl
    {
    };

    template <typename R, typename... Args>
    struct callable_arguments_impl<std::function<R(Args...)>>
    {
        using type = std::tuple<std::decay_t<Args>...>;
    };

} // namespace memoization::details

namespace memoization
{
    template <typename T>
    struct remove_cvref : std::remove_cv<std::remove_reference_t<T>>
    {
    };

    template <typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    template <typename T>
    using AsFunction = decltype(std::function{std::declval<T>()});

#if defined(__GNUC__) || defined(__GNUG__)
// CTAD bug in gcc, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98077
#define AS_FUNCTION(T) decltype(std::function{std::declval<T>()})
#else
#define AS_FUNCTION(T) AsFunction<T>
#endif

    template <typename T, typename = void>
    struct is_callable : std::false_type
    {
    };

    template <typename T>
    struct is_callable<T, std::void_t<AS_FUNCTION(T)>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr auto is_callable_v = is_callable<T>::value;

    template <typename T>
    struct callable_to_function
    {
        static_assert(is_callable_v<T>);
        using type = AS_FUNCTION(T);
    };

    template <typename T>
    using callable_to_function_t = typename callable_to_function<T>::type;

    template <typename T>
    struct callable_result
        : memoization::details::callable_result_impl<callable_to_function_t<T>>
    {
    };

    template <typename T>
    using callable_result_t = typename callable_result<T>::type;

    template <typename T>
    struct callable_arguments
        : memoization::details::callable_arguments_impl<callable_to_function_t<T>>
    {
    };

    template <typename T>
    using callable_arguments_t = typename callable_arguments<T>::type;

} // namespace memoization

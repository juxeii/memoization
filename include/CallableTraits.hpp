#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

namespace memoization::details
{
    template <typename T>
    struct function_components
    {
    };

    template <typename R, typename... Args>
    struct function_components<std::function<R(Args...)>>
    {
        using type = std::tuple<R(Args...), R, std::tuple<std::decay_t<Args>...>>;
    };

    template <typename T>
    using function_components_t = typename function_components<T>::type;

    template <typename T, auto I>
    using get_function_component_t = std::tuple_element_t<I, function_components_t<T>>;

    template <typename T>
    using function_signature_t = get_function_component_t<T, 0>;

    template <typename T>
    using function_result_t = get_function_component_t<T, 1>;

    template <typename T>
    using function_arguments_t = get_function_component_t<T, 2>;

} // namespace memoization::details

namespace memoization
{
    using namespace memoization::details;

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
    using callable_result_t = function_result_t<callable_to_function_t<T>>;

    template <typename T>
    using callable_arguments_t = function_arguments_t<callable_to_function_t<T>>;

    template <typename T>
    using remove_cvref_t = typename std::remove_cv<std::remove_reference_t<T>>::type;

} // namespace memoization

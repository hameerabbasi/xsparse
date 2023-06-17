//
// Created by Hameer Abbasi on 19.02.21.
//

#ifndef XSPARSE_TEMPLATE_UTILS_H
#define XSPARSE_TEMPLATE_UTILS_H
#include <tuple>
#include <type_traits>

namespace xsparse::util
{
    template <class T, template <class...> class TT>
    struct is_specialization_of : std::false_type
    {
    };

    template <template <class...> class TT, class... Ts>
    struct is_specialization_of<TT<Ts...>, TT> : std::true_type
    {
    };

    template <class T, template <class...> class TT>
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, TT>::value;

    template <class>
    struct is_tuple_with_integral_template_arguments : std::false_type
    {
    };

    template <class... Ts>
    struct is_tuple_with_integral_template_arguments<std::tuple<Ts...>>
        : std::bool_constant<(... && std::is_integral_v<Ts>)>
    {
    };

    template <class T>
    inline constexpr bool is_tuple_with_integral_template_arguments_v
        = is_tuple_with_integral_template_arguments<T>::value;

    template <typename Fn, typename Argument, std::size_t... Ns>
    auto tuple_transform_impl(Fn&& fn, Argument&& argument, std::index_sequence<Ns...>)
    {
        /**
         * @brief Apply a function to each element of a tuple during compile time.
         *
         * @tparam Fn - The function to apply to each element of the tuple.
         * @tparam Argument - The tuple to apply the function to.
         * @tparam Ns - Parameter pack of the indices of the tuple.
         */
        if constexpr (sizeof...(Ns) == 0)
            return std::tuple<>();  // empty tuple
        else if constexpr (std::is_same_v<decltype(fn(std::get<0>(argument))), void>)
        {
            (fn(std::get<Ns>(argument)), ...);  // no return value expected
            return;
        }
        // then dispatch lvalue, rvalue ref, temporary
        else if constexpr (std::is_lvalue_reference_v<decltype(fn(std::get<0>(argument)))>)
        {
            return std::tie(fn(std::get<Ns>(argument))...);
        }
        else if constexpr (std::is_rvalue_reference_v<decltype(fn(std::get<0>(argument)))>)
        {
            return std::forward_as_tuple(fn(std::get<Ns>(argument))...);
        }
        else
        {
            return std::tuple(fn(std::get<Ns>(argument))...);
        }
    }

    template <typename Fn, typename... Ts>
    auto tuple_transform(Fn&& fn, const std::tuple<Ts...>& tuple)
    {
        /**
         * @brief Apply a function to each element of a tuple during compile time.
         *
         * @details This function is a wrapper around tuple_transform_impl.
         * See:
         * https://codereview.stackexchange.com/questions/193420/apply-a-function-to-each-element-of-a-tuple-map-a-tuple
         *
         * @tparam Fn - The function to apply to each element of the tuple.
         * @tparam Ts - The types of the tuple.
         */
        return tuple_transform_impl(
            std::forward<Fn>(fn), tuple, std::make_index_sequence<sizeof...(Ts)>());
    }
}

#endif  // XSPARSE_TEMPLATE_UTILS_H

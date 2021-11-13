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
}
#endif  // XSPARSE_TEMPLATE_UTILS_H

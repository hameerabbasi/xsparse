//
// Created by Hameer Abbasi on 19.02.21.
//

#ifndef XSPARSE_TEMPLATE_UTILS_H
#define XSPARSE_TEMPLATE_UTILS_H
#include <tuple>
#include <type_traits>

#include <iostream>

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

    // Function to apply when the condition is met
    template <typename T>
    void applyFunction(const T& value) {
        std::cout << " got em... " << value << " ";
    }

    // Apply the function to the element if the mask value is true
    template <typename Function, typename T>
    inline void apply_if(const Function& function, const T& element, bool mask_value) {
        if (mask_value) {
            function(element);
        }
    }

    // Apply the function to each element in the tuple based on the mask
    template <typename Function, typename... Args, typename... MaskArgs, std::size_t... Indices>
    inline void apply_to_tuple_helper(
        const Function& function,
        const std::tuple<Args...>& tuple,
        const std::tuple<MaskArgs...>& mask,
        std::index_sequence<Indices...>
    ) noexcept {
        (apply_if(function, std::get<Indices>(tuple), std::get<Indices>(mask)), ...);
    }

    // TODO:
    // 1. Can we replace `MaskArgs` with a boolean template parameter?
    // 2. 
    // Apply the function to each element in the tuple if the corresponding mask element is true
    template <typename Function, typename... Args, typename... MaskArgs>
    inline void apply_to_tuple(
        const Function& function,
        const std::tuple<Args...>& tuple,
        const std::tuple<MaskArgs...>& mask
    ) noexcept {
        apply_to_tuple_helper(function, tuple, mask, std::index_sequence_for<Args...>{});
    }
}

#endif  // XSPARSE_TEMPLATE_UTILS_H

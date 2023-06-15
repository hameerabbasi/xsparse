//
// Created by Hameer Abbasi on 29.03.21.
//

#ifndef XSPARSE_LOCATE_HPP
#define XSPARSE_LOCATE_HPP

#include <type_traits>

#include <xsparse/util/base_traits.hpp>

/**
 * @brief The following code is used to determine if a class T has a member function locate().
 *
 * @tparam T - The class to check for the member function locate().
 *
 * @example
 *
 * struct A
 * {
 *    void locate() {}
 * };
 *
 * has_locate_v<A> // true
 */

template <class T, class = void>
struct has_locate : std::false_type
{
};

// template <class T>
// struct has_locate<T, decltype(T::locate)> : std::true_type
// {
// };
template <class T>
struct has_locate<T, std::void_t<decltype(&T::locate)>> : std::true_type
{
};

template <class T>
inline constexpr bool has_locate_v = has_locate<T>::value;

#endif  // XSPARSE_LOCATE_HPP

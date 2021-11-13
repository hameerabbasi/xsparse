//
// Created by Hameer Abbasi on 29.03.21.
//

#ifndef XSPARSE_LOCATE_HPP
#define XSPARSE_LOCATE_HPP

#include <type_traits>

template <class T, class = void>
struct has_locate : std::false_type
{
};

template <class T>
struct has_locate<T, decltype(T::locate)> : std::true_type
{
};

template <class T>
inline constexpr bool has_locate_v = has_locate<T>::value;

#endif  // XSPARSE_LOCATE_HPP

#ifndef XSPARSE_UTIL_BASE_TRAITS_HPP
#define XSPARSE_UTIL_BASE_TRAITS_HPP
#include <tuple>
#include <cstdint>

namespace xsparse::util
{
    template <template <class...> class T, class LowerLevels, class... Opts>
    struct base_traits;

    template <class T>
    struct coordinate_position_trait
    {
        using Coordinate = typename T::Coordinate;
        using Position = typename T::Position;
    };

    template <class T>
    using coordinate_t = typename coordinate_position_trait<T>::Coordinate;

    template <class T>
    using position_t = typename coordinate_position_trait<T>::Position;

    template <template <class...> class T, class... LowerLevels, class... Opts>
    struct base_traits<T, std::tuple<LowerLevels...>, Opts...>
    {
        using Level = T<std::tuple<LowerLevels...>, Opts...>;
        using I = typename std::tuple<coordinate_t<LowerLevels>...>;
        using P = typename std::tuple<position_t<LowerLevels>...>;
        using IK = coordinate_t<Level>;
        using PK = position_t<Level>;
        static_assert(std::is_convertible_v<IK, uintptr_t>,
                      "`IK` must be convertible to uintptr_t.");
        static_assert(std::is_convertible_v<PK, uintptr_t>,
                      "`PK` must be convertible to uintptr_t.");
        using PKM1 = std::tuple_element_t<
            // First element of P
            0,
            // Except that the type list has uint8_t appended to it.
            // This has the effect of choosing uint8_t instead of the
            // first element if `sizeof...(LowerLevels) == 0`.
            // is in place.
            std::tuple<position_t<LowerLevels>..., bool>>;
    };
}

#endif
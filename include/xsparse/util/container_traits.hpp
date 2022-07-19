#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace xsparse::util
{
    template <template <class...> class T,
              class LowerLevels,
              class IK,
              class PK,
              class... Containers>
    struct container_traits;

    template <template <class...> class T,
              class... LowerLevels,
              class IK,
              class PK,
              class... Containers>
    struct container_traits<T, std::tuple<LowerLevels...>, IK, PK, Containers...>
    {
        using Vector_t = typename std::vector;
        using Unordered_set_t = typename std::unordered_set;
        using Unordered_map_t = typename std::unordered_map;
    };
}

#endif
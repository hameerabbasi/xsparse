#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace xsparse::util
{
    template <class TVec, class TSet, class TMap>
    struct container_traits;

    template <class TVec = typename std::vector,
              class TSet = typename std::unordered_set,
              class TMap = typename std::unordered_map>
    struct container_traits<TVec, TSet, TMap>
    {
        using Vec = TVec;
        using Set = TSet;
        using Map = TMap;
    };
}

#endif
#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace xsparse::util
{
    template <class IK,
              class PK,
              template <typename> class TVec,
              template <typename> class TSet,
              template <typename, typename> class TMap>
    struct container_traits
    {
        using Vec = TVec<IK>;
        using Set = TSet<IK>;
        using Map = TMap<IK, PK>;
    };
}

#endif
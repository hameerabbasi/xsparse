#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <set>
#include <unordered_map>

namespace xsparse::util
{
    template <template <class...> class T, class LowerLevels, class IK, class PK, class... Containers>
    struct container_traits;

    template <template <class...> class T, class... LowerLevels, class IK, class PK, class... Containers>
    struct container_traits<T, std::tuple<LowerLevels...>, IK, PK, Containers...>
    {
        using Vector_pos_t = typename std::vector<PK>;
        using Vector_crd_t = typename std::vector<IK>;
        using Set_pos_t = typename std::set<PK>;
        using Set_crd_t = typename std::set<IK>;
        using Unordered_map_t = typename std::unordered_map<IK, PK>;
    };
}

#endif
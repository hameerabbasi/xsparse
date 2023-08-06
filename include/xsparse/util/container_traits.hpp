#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace xsparse::util
{
    template <template <class...> class TVec,
              template <class...>
              class TSet,
              template <class...>
              class TMap>
    struct container_traits
    {
        template <class Elem>
        using Vec = TVec<Elem>;

        template <class Elem>
        using Set = TSet<Elem>;

        template <class Key, class Val>
        using Map = TMap<Key, Val>;

        // Check that the container traits have the properly defined methods and method signatures
        // uses sample type inputs to check the signatures
        static_assert(
            std::is_same_v<decltype(std::declval<Vec<double>>().push_back(std::declval<double>())),
                           void>,
            "Vec must have `push_back` method with the correct signature.");

        static_assert(std::is_same_v<decltype(std::declval<Vec<double>>().resize(
                                         std::declval<typename Vec<double>::size_type>())),
                                     void>,
                      "Vec must have `resize` method with the correct signature.");

        static_assert(
            std::is_same_v<decltype(std::declval<Vec<double>>()[std::declval<std::size_t>()]),
                           decltype(std::declval<Vec<double>>().at(std::declval<std::size_t>()))>,
            "Vec must have `operator[]` method with the correct signature.");

        static_assert(std::is_same_v<decltype(std::declval<Map<int, std::size_t>>().find(
                                         std::declval<std::size_t>())),
                                     typename Map<int, std::size_t>::iterator>,
                      "Map must have `find` method with the correct signature.");

        static_assert(std::is_same_v<decltype(std::declval<Set<std::size_t>>().contains(
                                         std::declval<std::size_t>())),
                                     bool>,
                      "Set must have `contains` method with the correct signature.");
    };
}

#endif

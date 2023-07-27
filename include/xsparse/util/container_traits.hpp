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

        // TODO:
        // implement generic methods for push_back, and operator[]
        // what methods would these be? would it be the following?
        // What should they do?
        template <typename size_type>
        constexpr void resize(size_type count)
        {
        }

        template <typename T>
        constexpr void push_back(const T& value)
        {
        }

        template <typename size_type, typename Elem>
        constexpr Elem operator[](size_type pos)
        {
        }
    };    
}

#endif
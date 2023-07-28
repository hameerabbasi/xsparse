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

        // Required member functions that must be implemented by the container traits.
        static void resize(std::size_t new_size);

        template <class Elem>
        static void push_back(const Elem& value);

        template <class Key, class Val>
        static Val find(const Key& key);

        // Overloaded operator[] for accessing elements by index.
        inline auto operator[](std::size_t index);
    };
}

#endif
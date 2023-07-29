#ifndef XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#define XSPARSE_UTIL_CONTAINER_TRAITS_HPP
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace xsparse::util
{
    // Helper type trait to check the signature of push_back method
    template <class Container, class Elem>
    struct has_push_back
    {
        template <typename C, typename E>
        static auto test(int)
            -> decltype(std::declval<C>().push_back(std::declval<E>()), std::true_type());

        template <typename, typename>
        static auto test(...) -> std::false_type;

        static constexpr bool value = decltype(test<Container, Elem>(0))::value;
    };

    // Helper type trait to check the signature of resize method
    template <class Container, class Size>
    struct has_resize
    {
        template <typename C, typename S>
        static auto test(int)
            -> decltype(std::declval<C>().resize(std::declval<S>()), std::true_type());

        template <typename, typename>
        static auto test(...) -> std::false_type;

        static constexpr bool value = decltype(test<Container, Size>(0))::value;
    };

    // Helper type trait to check the signature of operator[] method
    template <class Container, class Index>
    struct has_operator_index
    {
        template <typename C, typename I>
        static auto test(int) -> decltype(std::declval<C>()[std::declval<I>()], std::true_type());

        template <typename, typename>
        static auto test(...) -> std::false_type;

        static constexpr bool value = decltype(test<Container, Index>(0))::value;
    };

    // Helper type trait to check if both push_back and resize are valid for the given container
    template <class Container, class Elem, class Index>
    struct has_valid_vec_methods
    {
        static constexpr bool value = has_push_back<Container, Elem>::value
                                      && has_resize<Container, typename Container::size_type>::value
                                      && has_operator_index<Container, Index>::value;
    };

    // Helper type trait to check the signature of find method
    template <class Container, class Key>
    struct has_find
    {
        template <typename C, typename K>
        static auto test(int)
            -> decltype(std::declval<C>().find(std::declval<K>()), std::true_type());

        template <typename, typename>
        static auto test(...) -> std::false_type;

        static constexpr bool value = decltype(test<Container, Key>(0))::value;
    };

    // Helper type trait to check the signature of find method
    template <class Container, class Key>
    struct has_contains
    {
        template <typename C, typename K>
        static auto test(int)
            -> decltype(std::declval<C>().contains(std::declval<K>()), std::true_type());

        template <typename, typename>
        static auto test(...) -> std::false_type;

        static constexpr bool value = decltype(test<Container, Key>(0))::value;
    };

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
            has_valid_vec_methods<Vec<double>, double, std::size_t>::value,
            "Vec must have `push_back`, `resize` and `operator[]` methods with the correct signatures.");
        static_assert(has_find<Map<int, std::size_t>, std::size_t>::value,
                      "Map must have find and resize methods with the correct signatures.");
        static_assert(has_contains<Set<std::size_t>, std::size_t>::value,
                      "Set must have contain methods with the correct signatures.");
    };
}

#endif

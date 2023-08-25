#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP
#include <vector>
#include <tuple>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <xsparse/level_capabilities/locate.hpp>
#include <xsparse/util/template_utils.hpp>

template <class... Levels>
struct all_ordered_or_have_locate;

// Base case: No levels left to check
template <>
struct all_ordered_or_have_locate<>
{
    static constexpr bool value = true;  // All levels have been checked
};

// Recursive case: Check the current level and continue with the rest
template <class Level, class... RemainingLevels>
struct all_ordered_or_have_locate<Level, RemainingLevels...>
{
    static constexpr bool value = Level::LevelProperties::is_ordered || has_locate_v<Level>
                                      ? all_ordered_or_have_locate<RemainingLevels...>::value
                                      : false;
};

// Helper function to check if a level is ordered
template <typename Level>
constexpr bool
is_level_ordered()
{
    return Level::LevelProperties::is_ordered;
}


namespace xsparse::level_capabilities
{
    /**
     * @brief The class template for Coiteration of level formats.
     *
     * @details Uses a generic function object F to compare elements
     * from different sequences at the same position and returns a tuple of the
     * minimum index and the corresponding elements from each sequence.
     *
     * @tparam F - A function object that is used to compare elements from different ranges.
     * Its input is a tuple of booleans corresponding to each of the levels. The output is a boolean
     * that is the result of the function.
     * @tparam IK - The type of the first element of each range.
     * @tparam PK - The type of the second element of each range.
     * @tparam Levels - A tuple of level formats, where each level is itself a tuple of elements
     * to be iterated.
     * @tparam Is - A tuple of indices that is used to keep track of the current position in each
     * level.
     *
     * @param levels - A tuple of levels is passed in during runtime via the constructor.
     */

    template <template <bool...> class F, class Ffunc, class IK, class PK, class Levels, class Is, class Ps>
    class Coiterate;

    // XXX: This double-passing of the function `F` and `Ffunc` is a workaround
    // to get a compile-time check of the possible boolean inputs and also to
    // work in operator!= during runtime. This can probably be made cleaner.
    template <template <bool...> class F,
              class Ffunc,
              class IK,
              class PK,
              class... Levels,
              class... Is,
              class... Ps>
    class Coiterate<F, Ffunc, IK, PK, std::tuple<Levels...>, std::tuple<Is...>, std::tuple<Ps...>>
    {
    private:
        Ffunc const m_comparisonHelper;
        std::tuple<Levels&...> const m_levelsTuple;

        // A tuple of booleans corresponding to each level, where true indicates that the level is
        // ordered. which is used in determining the template recursion
        static constexpr auto ordered_mask_tuple = std::make_tuple(is_level_ordered<Levels>()...);

        template <std::size_t I, bool... Args>
        static constexpr auto validate_boolean_helper()
        /**
         * @brief Helper function to recursively construct a tuple of boolean arguments to be
         * evaluated with the function object F.
         */
        {
            // Check if the current recursion depth (I) is less than the number of elements in Mask.
            if constexpr (I > 0)
            {
                // If the Ith element of Mask is not ordered (false), branch into two paths.
                if constexpr (std::get<I - 1>(ordered_mask_tuple) == false)
                {
                    // Next, set the Ith element of f_args to false and recursively call the
                    // function.
                    validate_boolean_helper<I - 1, true, Args...>();
                }
                // If the Ith element of Mask is ordered (true), only branch into one path with the
                // Ith element set to false.
                validate_boolean_helper<I - 1, false, Args...>();
            }
            else
            {
                // When we reach the end of recursion (base case), call the function object F with
                // the constructed boolean arguments.
                static_assert(sizeof...(Args) == sizeof...(Levels),
                              "Number of arguments must be equal to number of levels");

                // perform check for correct conjunctive/disjunctive merges
                static_assert(F<Args...>::value == false,
                              "Function F should return false for the given arguments.");
            }
        }

    public:
        explicit constexpr inline Coiterate(Ffunc f, Levels&... levels)
            : m_comparisonHelper(f)
            , m_levelsTuple(std::tie(levels...))
        {
            if (![](auto const& first, auto const&... rest)
                { return ((first == rest) && ...); }(levels.size()...))
            {
                throw std::invalid_argument("level sizes should be same");
            }

            // the following checks that at least one level is always ordered,
            // and any unordered levels are only part of conjunctive merges
            // check that at least one of the levels is ordered
            constexpr bool check_ordered = (Levels::LevelProperties::is_ordered || ...);
            static_assert(check_ordered,
                          "Coiteration is only allowed if at least one level is ordered");

            // check that all levels are either ordered, or has locate function
            constexpr bool check_levels = all_ordered_or_have_locate<Levels...>::value;
            static_assert(
                check_levels,
                "Coiteration is only allowed if all levels are ordered or have the locate function");

            // Add a member variable to hold the boolean mask of ordered levels
            static_assert(std::get<0>(ordered_mask_tuple) == true, "First level must be ordered");
            static_assert(sizeof...(Levels) == std::tuple_size_v<decltype(ordered_mask_tuple)>,
                          "Size of ordered mask tuple must be equal to number of levels");

            // check that the comparison helper defines a disjunctive merge only over ordered levels
            // recursively pass in false, and true for each level to `m_comparisonHelper` for each
            // unordered level, ans pass in false for each ordered level.
            validate_boolean_helper<sizeof(ordered_mask_tuple)>();
        }

    public:
        class coiteration_helper
        {
        public:
            Coiterate const& m_coiterate;
            std::tuple<Is...> const m_i;
            std::tuple<Ps...> const m_pkm1;
            std::tuple<typename Levels::iteration_helper...> m_iterHelpers;

        public:
            explicit inline coiteration_helper(Coiterate const& coiterate,
                                               std::tuple<Is...> i,
                                               std::tuple<Ps...> pkm1) noexcept
                : m_coiterate(coiterate)
                , m_i(std::move(i))
                , m_pkm1(std::move(pkm1))
                , m_iterHelpers(std::apply([&](auto&... args)
                                           { return std::make_tuple(
                                            args.iter_helper(std::get<decltype(args)::value_type>(i),
                                                             std::get<decltype(args)::value_type>(pkm1))...); },
                                           coiterate.m_levelsTuple))
            {
            }

            class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<typename Levels::iteration_helper::iterator...> iterators;
                IK min_ik;

            private:
                template <typename... iter1, typename... iter2>
                inline constexpr auto compareHelper(const std::tuple<iter1...>& it1,
                                                    const std::tuple<iter2...>& it2) const noexcept
                {
                    static_assert(sizeof...(iter1) == sizeof...(iter2));

                    return compare(it1, it2, std::make_index_sequence<sizeof...(iter1)>{});
                }

                template <typename... iter1, typename... iter2, std::size_t... I>
                inline constexpr auto compare(const std::tuple<iter1...>& it1,
                                              const std::tuple<iter2...>& it2,
                                              std::index_sequence<I...>) const noexcept
                {
                    return std::tuple{ compare_level(std::get<I>(it1), std::get<I>(it2))... };
                }

                template <typename iter1, typename iter2>
                inline constexpr auto compare_level(iter1& it1, iter2& it2) const noexcept
                {
                    if constexpr (iter1::parent_type::LevelProperties::is_ordered)
                    {
                        // check if the current position is equal to the end
                        return it1 == it2;
                    }
                    else
                    {
                        // always return true for unordered levels
                        return true;
                    }
                }

                template <std::size_t I>
                inline constexpr IK get_min_ik_level() const noexcept
                {
                    using iter_type = std::tuple_element_t<I, decltype(iterators)>;
                    iter_type it_current = std::get<I>(iterators);
                    iter_type it_end = std::get<I>(m_coiterHelper.m_iterHelpers).end();

                    static_assert(iter_type::parent_type::LevelProperties::is_ordered
                                      || has_locate_v<typename iter_type::parent_type>,
                                  "The level must be ordered or have a locate function.");

                    if constexpr (iter_type::parent_type::LevelProperties::is_ordered)
                    {
                        return it_current != it_end ? std::get<0>(*it_current) : min_ik;
                    }
                    else
                    {
                        return std::numeric_limits<IK>::max();
                    }
                }

                template <std::size_t... I>
                inline constexpr void calc_min_ik([[maybe_unused]] std::index_sequence<I...> i)
                /**
                 * @brief Calculate the minimum index from a tuple of elements based on comparison
                 * and conditions.
                 *
                 * @tparam I... - index sequence.
                 * @param i - index sequence that is unused.
                 *
                 * @details This function gets the minimum index from all levels.
                 */
                {
                    min_ik = std::min({ get_min_ik_level<I>()... });
                }

                inline constexpr void min_helper()
                {
                    calc_min_ik(std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>{});
                }

                template <class iter, std::size_t I>
                inline constexpr auto get_PK_iter(iter& i) const noexcept
                /**
                 * @brief Get the PK of the iterator at index I.
                 *
                 * @tparam iter - The type of the iterator.
                 * @tparam I - The index of the iterator in the tuple of iterators. The template
                 * param is only used in the setting where the iterator is unordered.
                 *
                 * @param i - The iterator passed by reference.
                 *
                 * @return The PK of the iterator at index I.
                 */
                {
                    // XXX: move this into a destructor.
                    // levels should either be ordered or have locate function.
                    static_assert(iter::parent_type::LevelProperties::is_ordered
                                      || has_locate_v<typename iter::parent_type>,
                                  "Levels should either be ordered or have locate function.");

                    if constexpr (iter::parent_type::LevelProperties::is_ordered)
                    {
                        return deref_PKs(i);
                    }
                    else if constexpr (has_locate_v<typename iter::parent_type>)
                    {
                        return std::get<I>(this->m_coiterHelper.m_coiterate.m_levelsTuple)
                            .locate(std::get<I>(m_coiterHelper.m_pkm1), min_ik);
                    }
                }

                template <std::size_t I>
                inline constexpr auto get_PKs_level() const noexcept
                {
                    using iter_type = std::tuple_element_t<I, decltype(iterators)>;
                    iter_type it(std::get<I>(iterators));
                    return get_PK_iter<iter_type, I>(it);
                }

                template <std::size_t... I>
                inline constexpr auto get_PKs_complete(
                    [[maybe_unused]] std::index_sequence<I...> i) const noexcept
                /**
                 * @brief Helper function to obtain PKs from each level's iterator.
                 *
                 * @details Performs a fold expression over `get_PKs_level` for every index in
                 * the index sequence `i`.
                 */
                {
                    return std::make_tuple(get_PKs_level<I>()...);
                }

                inline constexpr auto get_PKs() const noexcept
                /**
                 * @brief Return tuple of PKs from each level.
                 *
                 * @details If the level is ordered, return the PK from the iterator using
                 * dereferencing `*iter`. If the level is unordered, return the PK from
                 * the iterator using `iter.locate()`.
                 */
                {
                    return get_PKs_complete(
                        std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>{});
                }

                template <class iter>
                inline auto deref_PKs(iter i) const noexcept
                {
                    return (std::get<0>(*i) == min_ik)
                               ? std::optional<std::tuple_element_t<1, decltype(*i)>>(
                                   std::get<1>(*i))
                               : std::nullopt;
                }

                template <class iter>
                inline void advance_iter(iter& i) const noexcept
                {
                    // advance iterator if it is ordered
                    if constexpr (iter::parent_type::LevelProperties::is_ordered)
                    {
                        if (static_cast<IK>(std::get<0>(*i)) == min_ik)
                        {
                            ++i;
                        }
                    }
                }

            public:
                using iterator_category = std::forward_iterator_tag;
                using reference = typename std::
                    tuple<IK, std::tuple<std::optional<typename Levels::BaseTraits::PK>...>>;

                explicit inline iterator(
                    coiteration_helper const& coiterHelper,
                    std::tuple<typename Levels::iteration_helper::iterator...> it) noexcept
                    : m_coiterHelper(coiterHelper)
                    , iterators(it)
                {
                    min_helper();
                }

                inline reference operator*() const noexcept
                {
                    auto PK_tuple = get_PKs();
                    return { min_ik, PK_tuple };
                }

                inline iterator operator++(int) const noexcept
                {
                    iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                inline iterator& operator++() noexcept
                {
                    std::apply([&](auto&... args) { ((advance_iter(args)), ...); }, iterators);
                    min_helper();
                    return *this;
                }

                inline bool operator!=(iterator const& other) const noexcept
                {
                    // a tuple of booleans E.g. (true, false, true, false)
                    const auto result_bools = compareHelper(iterators, other.iterators);
                    return !m_coiterHelper.m_coiterate.m_comparisonHelper(result_bools);
                };

                inline bool operator==(iterator const& other) const noexcept
                {
                    return !(*this != other);
                };
            };

            inline iterator begin() const noexcept
            {
                return iterator{ *this,
                                 std::apply([&](auto&... args)
                                            { return std::tuple(args.begin()...); },
                                            this->m_iterHelpers) };
            }

            inline iterator end() const noexcept
            {
                return iterator{ *this,
                                 std::apply([&](auto&... args)
                                            { return std::tuple(args.end()...); },
                                            this->m_iterHelpers) };
            }
        };

        coiteration_helper coiter_helper(std::tuple<Is...> i, std::tuple<Ps...> pkm1)
        /**
         * @brief 
         * 
         * IK can be a single min_ik if we are coiterating over the same coordinate each level
         * 
         * IK
         */
        {
            return coiteration_helper{ *this, i, pkm1 };
        }
    };
}

#endif

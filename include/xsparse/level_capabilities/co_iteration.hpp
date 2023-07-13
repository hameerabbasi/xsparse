#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP
#include <vector>
#include <tuple>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <xsparse/level_capabilities/locate.hpp>
#include <xsparse/util/template_utils.hpp>

#include <iostream>


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


template <class... Levels>
struct TupleEvaluator {
    template <class F>
    static constexpr bool Evaluate(F&& f) {
        return EvaluateHelper<sizeof...(Levels), F>::template Evaluate<Levels...>(std::forward<F>(f));
    }

    private:
        template <std::size_t N, class F>
        struct EvaluateHelper {
            template <class... Args>
            static constexpr bool Evaluate(F&& f) {
                if constexpr (std::tuple_element_t<N - 1, std::tuple<Levels...>>::LevelProperties::is_ordered)
                {
                    return EvaluateHelper<N - 1, F>::template Evaluate<Args..., false>(std::forward<F>(f));
                }
                else
                {
                    return EvaluateHelper<N - 1, F>::template Evaluate<Args..., false>(std::forward<F>(f)) &&
                        EvaluateHelper<N - 1, F>::template Evaluate<Args..., true>(std::forward<F>(f));
                }
            }
        };

        template <class F>
        struct EvaluateHelper<0u, F> {
            template <class... Args>
            static constexpr bool Evaluate(F&& f) {
                return std::invoke(std::forward<F>(f), Args{}...);
            }
        };
};


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
     * @note Coiteration is only allowed through tuples of levels if the following criterion is met:
     * If:
     * 1. the levels are all ordered (i.e. has the `is_ordered == True` property)
     * 2. if any of the level are do not have the is_ordered property, it must have the locate
     * function, else return False. Then do a check that `m_comparisonHelper` defines
     * a conjunctive merge (i.e. AND operation).
     * Otherwise, coiteration is not allowed.
     * 
     * Now, I want to do a check over the levels and `F`that defines a valid coiteration. F is
     * a function that takes in boolean values corresponding to each level and returns a boolean.
     * For example, if there are 5 levels, then F takes in a tuple of 5 booleans and returns a
     * boolean.
     * 
     * For each ordered level, we want to pass in `false` to F, and for each unordered level, we want to
     * evaluate `F` with `true` and `false` for the unordered level. I want to implement a template
     * recursion over the `Levels` tuple that does this check. Initially `F` is evaluated
     * with all inputs as `false`. Then recursing over `Levels`, if a level is unordered,
     * we want to evaluate `F` with `true` and `false` for that level. If a level is ordered,
     * we want to evaluate `F` with `false` for that level. For any level
     * that we are not on, the input to `F` is `false`. If `F` returns `true` for any of these
     * evaluations, then we throw an error. 
     * 
     * For example, if Levels = (A, B, C, D)
     
        And A, and D are unordered with locate() function defined and B and C are ordered.

        We want to check that the output is always false for F evaluated:

        F(false, false, false, false)
        F(true, false, false, false)
        F(false, false, false, true)

        If any of the above returns `true`, then we throw an error.

     *
     * This check is done automatically in the constructor, via the function `
     */

    template <class F, class IK, class PK, class Levels, class Is>
    class Coiterate;

    template <class F, class IK, class PK, class... Levels, class... Is>
    class Coiterate<F, IK, PK, std::tuple<Levels...>, std::tuple<Is...>>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;
        F const m_comparisonHelper;

    public:
        explicit inline Coiterate(F f, Levels&... levels)
            : m_levelsTuple(std::tie(levels...))
            , m_comparisonHelper(f)
        {
            if (![](auto const& first, auto const&... rest)
                { return ((first == rest) && ...); }(levels.size()...))
            {
                throw std::invalid_argument("level sizes should be same");
            }

            // the following checks that at least one level is always ordered,
            // and any unordered levels are only part of conjunctive merges
            // check that at least one of the levels is ordered
            static constexpr bool check_ordered = (Levels::LevelProperties::is_ordered || ...);
            static_assert(check_ordered, "Coiteration is only allowed if at least one level is ordered");

            // check that all levels are either ordered, or has locate function
            static constexpr bool check_levels = all_ordered_or_have_locate<Levels...>::value;
            static_assert(
                check_levels,
                "Coiteration is only allowed if all levels are ordered or have the locate function");

            // check that the comparison helper defines a disjunctive merge only over ordered levels
            // recursively pass in false, and true for each level to `m_comparisonHelper` for each
            // unordered level, ans pass in false for each ordered level.
            // check that m_comparisonHelper(true, true, ..., false, ...) == false
            // index sequence for all the levels, index sequence of the unordered property
            // constexpr bool result = evaluateLevels(f, );
            // static constexpr bool check_F = evaluateLevels<Levels...>();
            // static_assert(!check_F, "F returns `true` for some evaluations");
            // check_coiterate_condition(f, levels);

            static constexpr bool check_coiterate_condition = TupleEvaluator<Levels...>::Evaluate(f);
            static_assert(check_coiterate_condition == false);
        }

        // constexpr void check_coiterate_condition(F f, Levels const&...) {
        //     // check that f(false, false, ..., false) == false
        //     static_assert(f(std::make_tuple((Levels{}, false)...)) == false);

        //     // define a lambda function
        //     [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        //         constexpr auto make_single_level_tuple = []<std::size_t I>(std::integral_constant<std::size_t, I>) constexpr {
        //             constexpr auto levels_tuple = ordered_level_mask<Levels>();
        //             constexpr auto false_tuple = std::make_tuple((Levels{}, false)...);
        //             return std::make_tuple(std::get<Is>(I == Is ? levels_tuple : false_tuple)...);
        //         };

        //         static_assert(((f(make_single_level_tuple(std::integral_constant<std::size_t, Is>{})) == false) && ...));
        //     }(std::index_sequence_for<Levels...>{});
        // }

    public:
        class coiteration_helper
        {
        public:
            Coiterate const& m_coiterate;
            std::tuple<Is...> const m_i;
            PK const m_pkm1;
            std::tuple<typename Levels::iteration_helper...> m_iterHelpers;

        public:
            explicit inline coiteration_helper(Coiterate const& coiterate,
                                               std::tuple<Is...> i,
                                               PK pkm1) noexcept
                : m_coiterate(coiterate)
                , m_i(std::move(i))
                , m_pkm1(std::move(pkm1))
                , m_iterHelpers(std::apply([&](auto&... args)
                                           { return std::tuple(args.iter_helper(i, pkm1)...); },
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
                            .locate(m_coiterHelper.m_pkm1, min_ik);
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
                    return !m_coiterHelper.m_coiterate.m_comparisonHelper(
                        compareHelper(iterators, other.iterators));
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

        coiteration_helper coiter_helper(std::tuple<Is...> i, PK pkm1)
        {
            return coiteration_helper{ *this, i, pkm1 };
        }
    };
}

#endif

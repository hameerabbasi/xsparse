#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>
#include <algorithm>
#include <stdexcept>

#include <xsparse/level_capabilities/locate.hpp>

namespace xsparse::level_capabilities
{
    /*
    The class template for Coiteration of level formats.

    Uses a generic function object F to compare elements
    from different sequences at the same position and returns a tuple of the
    minimum index and the corresponding elements from each sequence.

    Parameters
    ----------
    F : class
        A function object that is used to compare two elements from different ranges.
    IK : class
        The type of the first element of each range.
    PK : class
        The type of the second element of each range.
    Levels : Tuple of class
        A tuple of level formats, where each level is itself a tuple of elements to be iterated.
    Is : Tuple of class
        A tuple of indices that is used to keep track of the current position in each level.

    Notes
    -----
    Coiteration is only allowed through tuples of levels if the following criterion is met:
    If:
        1. the levels are all ordered (i.e. has the `is_ordered == True` property)
        2. if any of the level are do not have the is_ordered property, it must have the locate
            function, else return False. Then do a check that `m_comparisonHelper` defines
            a conjunctive merge (i.e. AND operation).
    Otherwise, coiteration is not allowed.
    */
    template <class F, class IK, class PK, class Levels, class Is>
    class Coiterate;

    template <class F, class IK, class PK, class... Levels, class... Is>
    class Coiterate<F, IK, PK, std::tuple<Levels...>, std::tuple<Is...>>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;     // tuple of levels
        F const m_comparisonHelper;                     // comparison function
        std::tuple<std::size_t...> m_orderedIndices;    // indices of ordered levels in `m_levelsTuple`

    public:
        explicit inline Coiterate(F f, Levels&... levels)
            : m_levelsTuple(std::tie(levels...))
            , m_comparisonHelper(f)
            , m_orderedIndices(meet_criteria(f, levels...))
        {
            if (![](auto const& first, auto const&... rest)
                { return ((first == rest) && ...); }(levels.size()...))
            {
                throw std::invalid_argument("level sizes should be same");
            }

            if (std::tuple_size_v<decltype(m_orderedIndices)> == 0)
            {
                throw std::invalid_argument("levels do not meet coiteration criteria");
            }
        }

        // Check if the levels meet the criteria.
        static constexpr auto meet_criteria(F f, Levels&... levels)
        {
            constexpr bool all_ordered = std::conjunction_v<levels.is_ordered...>;
            constexpr bool any_locate = std::disjunction_v<has_locate_v<levels...>...>;
            constexpr bool is_conjunctive_merge
                = std::is_same_v<bool, decltype(f(std::declval<bool>(), std::declval<bool>()))>;

            // return a index sequence of the ordered levels
            if constexpr (all_ordered)
            {
                return std::index_sequence<Is...>{};
            }
            else if constexpr (any_locate && is_conjunctive_merge)
            {
                return std::index_sequence<(Levels::is_ordered ? Is : -1)...>{};
            }
            else
            {
                // levels do not meet coiteration criteria because either:
                // i) not all levels are ordered, or 
                // ii) one of the levels does not have the locate function
                return std::index_sequence<>{};
            }
        }

    public:
        class coiteration_helper
        {
        public:
            Coiterate const& m_coiterate;
            std::tuple<Is...> const m_i;
            PK const m_pkm1;
            std::tuple<typename Levels::LevelCapabilities::iteration_helper...> m_iterHelpers;
            std::tuple<std::size_t...> const& m_orderedIndices;  // Reference to ordered indices

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
                , m_orderedIndices(coiterate.m_orderedIndices)
            {
            }

            class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<typename Levels::LevelCapabilities::iteration_helper::iterator...>
                    iterators;
                IK min_ik;

            private:
                template <typename... T1, typename... T2>
                inline constexpr auto compareHelper(const std::tuple<T1...>& t1,
                                                    const std::tuple<T2...>& t2) const noexcept
                {
                    static_assert(sizeof...(T1) == sizeof...(T2));

                    return compare(t1, t2, std::make_index_sequence<sizeof...(T1)>{});
                }

                template <typename... T1, typename... T2, std::size_t... I>
                inline constexpr auto compare(const std::tuple<T1...>& t1,
                                              const std::tuple<T2...>& t2,
                                              std::index_sequence<I...>) const noexcept
                {
                    return std::tuple{ std::get<I>(t1) == std::get<I>(t2)... };
                }

                template <typename... T1, typename... T2>
                inline constexpr void min_helper(const std::tuple<T1...>& t1,
                                                 const std::tuple<T2...>& t2)
                {
                    static_assert(sizeof...(T1) == sizeof...(T2));

                    calc_min_ik(t1, t2, std::make_index_sequence<sizeof...(T1)>{});
                }

                template <typename... T1, typename... T2, std::size_t... I>
                inline constexpr void calc_min_ik(const std::tuple<T1...>& t1,
                                                  const std::tuple<T2...>& t2,
                                                  std::index_sequence<I...>)
                {
                    min_ik = std::min({ (std::get<I>(t1) != std::get<I>(t2))
                                            ? std::get<0>(*std::get<I>(t1))
                                            : min_ik... });
                }

            public:
                using iterator_category = std::forward_iterator_tag;
                using reference = typename std::
                    tuple<IK, std::tuple<std::optional<typename Levels::BaseTraits::PK>...>>;

                explicit inline iterator(
                    coiteration_helper const& coiterHelper,
                    std::tuple<typename Levels::LevelCapabilities::iteration_helper::iterator...>
                        it) noexcept
                    : m_coiterHelper(coiterHelper)
                    , iterators(it)
                {
                    min_helper(iterators,
                               std::apply([&](auto&... args) { return std::tuple(args.end()...); },
                                          m_coiterHelper.m_iterHelpers));
                }

                inline auto get_PKs() const noexcept
                {
                    return std::apply([&](auto&... args)
                                      { return std::make_tuple(locate(args)...); },
                                      this->iterators);
                }

                template <class iter>
                inline auto locate(iter i) const noexcept
                {
                    return (std::get<0>(*i) == min_ik)
                               ? std::optional<std::tuple_element_t<1, decltype(*i)>>(
                                   std::get<1>(*i))
                               : std::nullopt;
                }

                template <class iter>
                inline void advance_iter(iter& i) const noexcept
                {
                    if (static_cast<IK>(std::get<0>(*i)) == min_ik)
                    {
                        ++i;
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
                    min_helper(iterators,
                               std::apply([&](auto&... args) { return std::tuple(args.end()...); },
                                          m_coiterHelper.m_iterHelpers));
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

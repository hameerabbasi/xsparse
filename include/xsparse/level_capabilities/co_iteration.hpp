#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>
#include <algorithm>
#include <stdexcept>
#include <xsparse/level_capabilities/locate.hpp>
#include <xsparse/util/template_utils.hpp>

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
        }

        template <std::size_t I>
        constexpr bool is_level_ordered() const noexcept
        /**
         * @brief Check if a single level is ordered or not at index I.
         */
        {
            return std::decay_t<decltype(std::get<I>(m_levelsTuple))>::LevelProperties::is_ordered;
        }

        template <std::size_t... I>
        constexpr auto ordered_level_mask_impl(std::index_sequence<I...>) const noexcept
        /**
         * @brief Template recursion to construct tuples of true/false indicating ordered/unordered
         * levels.
         */
        {
            return std::make_tuple(is_level_ordered<I>()...);
        }

        constexpr auto ordered_level_mask() const noexcept
        /**
         * @brief Compute a tuple of true/false indicating ordered/unordered levels.
         *
         * @details If all levels are ordered, return a tuple of true. If any of the levels
         * are unordered, return a tuple of true/false, where the true/false indicates
         * ordered/unordered levels. Also does a compiler-time check that the levels meet the
         * coiteration criteria given the function object `f`. This function IS compiler-evaluated.
         *
         * @return A tuple of true/false indicating ordered/unordered levels.
         */
        {
            return ordered_level_mask_impl(std::index_sequence_for<Levels...>());
        }

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
                    std::tuple<typename Levels::iteration_helper::iterator...> it) noexcept
                    : m_coiterHelper(coiterHelper)
                    , iterators(it)
                {
                    min_helper(iterators,
                               std::apply([&](auto&... args) { return std::tuple(args.end()...); },
                                          m_coiterHelper.m_iterHelpers));
                }

                template <std::size_t I>
                inline std::enable_if_t<
                    std::tuple_element_t<I, decltype(iterators)>::parent_type::
                        LevelProperties::is_ordered,
                    std::optional<typename std::tuple_element_t<I,
                                                  decltype(iterators)>::parent_type::BaseTraits::PK>>
                get_PKs_level() const noexcept
                {
                    return deref_PKs(std::get<I>(iterators));
                }

                template <std::size_t I>
                inline std::enable_if_t<
                    !std::tuple_element_t<I, decltype(iterators)>::parent_type::
                            LevelProperties::is_ordered
                        && has_locate_v<
                            typename std::tuple_element_t<I,
                                                          decltype(iterators)>::parent_type>,
                    std::optional<typename std::tuple_element_t<I,
                                                  decltype(iterators)>::parent_type::BaseTraits::PK>>
                get_PKs_level() const noexcept
                {
                    return std::get<I>(this->m_coiterHelper.m_coiterate.m_levelsTuple)
                        .locate(m_coiterHelper.m_pkm1, min_ik);
                }

                template <std::size_t... I>
                inline auto get_PKs_complete([[maybe_unused]] std::index_sequence<I...> i) const noexcept
                {
                    return std::make_tuple(get_PKs_level<I>()...);
                }

                inline auto get_PKs() const noexcept
                {
                    /**
                     * @brief Return tuple of PKs from each level.
                     *
                     * @details If the level is ordered, return the PK from the iterator using
                     * dereferencing `*iter`. If the level is unordered, return the PK from
                     * the iterator using `iter.locate()`.
                     */

                    // return std::apply(
                    //     [&](
                    //         auto&... args
                    //     )
                    //     {
                    //         return std::make_tuple(get_PKs_level()...);
                    //     }, std::integral_constant<std::size_t,
                    //     std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>>{}
                    // );
                    // return std::apply(
                    //     [&](auto&... args)
                    //     {
                    //         return std::make_tuple(get_PKs_level()...);
                    //     },
                    //     std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>
                    // );
                    return get_PKs_complete(std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>{});

                    // return tuple_transform(
                    //     get_PKs_complete(),
                    //     // this->iterators,
                    //     std::make_tuple(std::make_index_sequence<std::tuple_size_v<decltype(iterators)>>{}...));

                    // OLD IMPLEMENTATION:
                    // return std::apply([&](auto&... args)
                    // {
                    //     return std::make_tuple(deref_PKs(args)...);
                    // }, this->iterators);
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
                    // TODO: only advance_iter(args) if the iterator corresponds to ordered level
                    // auto levelMask = m_coiterHelper.m_coiterate.ordered_level_mask();
                    // std::size_t i = 0;
                    // std::apply([&](auto&... args) {
                    //     ((std::get<i++>(levelMask) ? advance_iter(args) : void()), ...);
                    // }, iterators);

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

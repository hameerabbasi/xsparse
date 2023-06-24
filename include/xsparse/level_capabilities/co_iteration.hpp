#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP
#include <iostream>
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
                /**
                 * @brief Calculate the minimum index from a tuple of elements based on comparison and conditions.
                 *
                 * @tparam T1... - Types of the elements in the first tuple. 
                 * @tparam T2... - Types of the elements in the second tuple. 
                 * @param t1 - The first tuple. This is generally the current
                 * position of the iterator.
                 * @param t2 - The second tuple. This is generally the end
                 * position of the iterator.
                 *
                 * @details This function compares the elements of `t1` and `t2` at each corresponding index,
                 * and calculates the minimum index based on certain conditions. The elements at the same index
                 * in `t1` and `t2` are compared using the `!=` operator, and if they are not equal, the element
                 * in `t1` is compared to the current value of `min_ik`. If it is less than the current `min_ik`,
                 * `min_ik` is updated to the element's value. The minimum index is returned after all comparisons.
                 */
                {
                    std::cout << "Number of elements in t1: " << sizeof...(T1) << std::endl;
                    std::cout << "Number of elements in t1: " << sizeof...(T1) << std::endl;
                    std::cout << "Index sequence I: ";
                    ((std::cout << I << ' '), ...); // Print each index
                    std::cout << std::endl;
                    std::cout << "t1: ";
                    ((std::cout << std::get<0>(*std::get<I>(t1)) << ' '), ...); // Print each index
                    std::cout << std::max({ (std::get<I>(t1) != std::get<I>(t2))
                                            ? std::get<0>(*std::get<I>(t1))
                                            : min_ik... }) << std::endl;
                    min_ik = std::min({ (std::get<I>(t1) != std::get<I>(t2))
                                            ? std::get<0>(*std::get<I>(t1))
                                            : min_ik... });
                    std::cout << "calc_min_ik after: " << min_ik << std::endl;
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

                // Apply the function to the element if the mask value is true
                template <typename Function, typename T>
                inline void apply_if(const Function& function, const T& element, bool mask_value) {
                    if (mask_value) {
                        function(element);
                    }
                }

                // Apply the function to each element in the tuple based on the mask
                template <typename Function, typename... Args, typename... MaskArgs, std::size_t... Indices>
                inline void apply_to_tuple_helper(
                    const Function& function,
                    const std::tuple<Args...>& tuple,
                    const std::tuple<MaskArgs...>& mask,
                    std::index_sequence<Indices...>
                ) noexcept {
                    (apply_if(function, std::get<Indices>(tuple), std::get<Indices>(mask)), ...);
                }

                // Apply the function to each element in the tuple if the corresponding mask element is true
                template <typename Function, typename... Args, typename... MaskArgs>
                inline void apply_to_tuple(
                    const Function& function,
                    const std::tuple<Args...>& tuple,
                    const std::tuple<MaskArgs...>& mask
                ) noexcept {
                    apply_to_tuple_helper(function, tuple, mask, std::index_sequence_for<Args...>{});
                }

                // Attempt 2 at apply_to_tuple
                // template <typename T, typename Function>
                // void apply_if(T&& value, Function&& func, std::true_type) {
                //     std::forward<Function>(func)(std::forward<T>(value));
                // }

                // template <typename T, typename Function>
                // void apply_if(T&&, Function&&, std::false_type) {
                //     // Do nothing when the mask value is false.
                // }

                // template <bool Condition, typename Tuple, typename MaskTuple, typename Function, std::size_t... Is>
                // void apply_if_impl(Tuple&& tuple, MaskTuple&& maskTuple, Function&& func, std::index_sequence<Is...>) {
                //     (apply_if(std::get<Is>(std::forward<Tuple>(tuple)),
                //             std::forward<Function>(func),
                //             std::get<Is>(std::forward<MaskTuple>(maskTuple))), ...);
                // }

                // template <bool Condition, typename Tuple, typename MaskTuple, typename Function>
                // void apply_if(Tuple&& tuple, MaskTuple&& maskTuple, Function&& func) {
                //     constexpr std::size_t TupleSize = std::tuple_size_v<std::decay_t<Tuple>>;
                //     apply_if_impl<Condition>(std::forward<Tuple>(tuple),
                //                             std::forward<MaskTuple>(maskTuple),
                //                             std::forward<Function>(func),
                //                             std::make_index_sequence<TupleSize>{});
                // }


                template <std::size_t I>
                inline std::enable_if_t<
                    std::tuple_element_t<I, decltype(iterators)>::parent_type::LevelProperties::
                        is_ordered,
                    std::optional<typename std::tuple_element_t<I, decltype(iterators)>::
                                      parent_type::BaseTraits::PK>>
                get_PKs_level() const noexcept
                {   
                    std::cout << "get_PKs_level_derf: " << I << std::endl;
                    return deref_PKs(std::get<I>(iterators));
                }

                template <std::size_t I>
                inline std::enable_if_t<
                    !std::tuple_element_t<I, decltype(iterators)>::parent_type::LevelProperties::
                            is_ordered
                        && has_locate_v<
                            typename std::tuple_element_t<I, decltype(iterators)>::parent_type>,
                    std::optional<typename std::tuple_element_t<I, decltype(iterators)>::
                                      parent_type::BaseTraits::PK>>
                get_PKs_level() const noexcept
                {
                    std::cout << "get_PKs_level: " << I << std::endl;
                    std::cout << "min_ik: " << min_ik << std::endl;
                    return std::get<I>(this->m_coiterHelper.m_coiterate.m_levelsTuple)
                        .locate(m_coiterHelper.m_pkm1, min_ik);
                }

                template <std::size_t... I>
                inline auto get_PKs_complete(
                    [[maybe_unused]] std::index_sequence<I...> i) const noexcept
                /**
                 * @brief Helper function to obtain PKs from each level's iterator.
                 *
                 * @details Calls overriden `get_PKs_level` function that are defined at
                 * compile-time for a ordered, or unordered level.
                 *
                 */
                {
                    return std::make_tuple(get_PKs_level<I>()...);
                }

                inline auto get_PKs() const noexcept
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
                    // ORIGINAL:
                    // std::apply([&](auto&... args) { ((advance_iter(args)), ...); }, iterators);
                    
                    // Apply advance_iter to iterators corresponding to true ordered_level_mask
                    // std::apply([&](auto&... args) {
                    //     applyMemberFunctionToIterators(iterators, m_coiterHelper.m_coiterate.ordered_level_mask()..., &decltype(args)::advance_iter);
                    // }, m_coiterHelper.m_iterHelpers);
                    // Apply advance_iter to iterators corresponding to true ordered_level_mask
                    // std::apply([&](auto&... args) {     
                    //     applyMemberFunctionToIterators(iterators, std::make_tuple(args.ordered_level_mask()...), &decltype(args)::advance_iter);
                    // }, m_coiterHelper.m_iterHelpers);
                    // Attempt 1: This doesn't work cuz of the i=0 runtime.
                    // auto levelMask = m_coiterHelper.m_coiterate.ordered_level_mask();
                    // std::size_t i = 0;
                    // std::apply([&](auto&... args) {
                    //     ((std::get<i++>(levelMask) ? advance_iter(args) : void()), ...);
                    // }, iterators);

                    // Attempt 2: This also doesn't work because I think hashed is a const_iterator..
                    auto lambda = [=](auto& i) { advance_iter(i); };
                    std::apply([&](auto&... args) { ((
                        apply_to_tuple(lambda, std::make_tuple(args), m_coiterHelper.m_coiterate.ordered_level_mask())
                    ), ...); }, iterators);

                    // Attempt 3: This for some reason doesn't work now because of the mask...
                    // std::apply([&](auto&&... args) { ((
                    //     apply_to_tuple([&](auto& iter) { advance_iter(iter); }, std::make_tuple(args), m_coiterHelper.m_coiterate.ordered_level_mask())
                    // ), ...); }, iterators);

                    // Attempt 4: This uses the attempt 2 of apply_to_tuple, but it doesn't work.
                    // Apply memberFunction only if the mask value is true
                    // apply_if<true>(iterators, m_coiterHelper.m_coiterate.ordered_level_mask(), [&obj](int value) {
                    //     advance_iter(value);
                    // });

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

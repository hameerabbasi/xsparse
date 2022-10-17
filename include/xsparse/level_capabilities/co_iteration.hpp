#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>
#include <algorithm>
#include <climits>
#include <initializer_list>

namespace xsparse::level_capabilities
{
    template <class F, class IK, class PK, class Levels, class Is>
    class Coiterate;

    template <class F, class IK, class PK, class... Levels, class... Is>
    class Coiterate<F, IK, PK, std::tuple<Levels...>, std::tuple<Is...>>
    {
    public:
        class coiteration_helper
        {
        public:
            std::tuple<Is...> const m_i;
            PK const m_pkm1;
            std::tuple<Levels&...> const m_levelsTuple;
            F const m_comparisonHelper;

        public:
            explicit inline coiteration_helper(F f,
                                               std::tuple<Is...>& i,
                                               PK& pkm1,
                                               Levels&... levels) noexcept
                : m_i(std::move(i))
                , m_pkm1(std::move(pkm1))
                , m_levelsTuple(std::tie(levels...))
                , m_comparisonHelper(f)
            {
                static_assert(std::tuple_size_v<decltype(m_levelsTuple)> >= 2,
                              "Tuple size should be at least 2");
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
                    min_ik = std::min(std::initializer_list{ (std::get<I>(t1) != std::get<I>(t2))
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
                    , min_ik(INT_MAX)
                {
                    auto ends = std::apply(
                        [&](auto&... args) {
                            return std::tuple(
                                args.iter_helper(m_coiterHelper.m_i, m_coiterHelper.m_pkm1)
                                    .end()...);
                        },
                        m_coiterHelper.m_levelsTuple);
                    min_helper(iterators, ends);
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
                    auto ends = std::apply(
                        [&](auto&... args) {
                            return std::tuple(
                                args.iter_helper(m_coiterHelper.m_i, m_coiterHelper.m_pkm1)
                                    .end()...);
                        },
                        m_coiterHelper.m_levelsTuple);
                    min_helper(iterators, ends);
                    return *this;
                }

                inline bool operator!=(iterator const& other) const noexcept
                {
                    return !(*this == other);
                };

                inline bool operator==(iterator const& other) const noexcept
                {
                    return m_coiterHelper.m_comparisonHelper(
                        compareHelper(iterators, other.iterators));
                };
            };

            inline iterator begin() const noexcept
            {
                auto static a = std::apply(
                    [&](auto&... args)
                    { return std::tuple(args.iter_helper(this->m_i, this->m_pkm1)...); },
                    this->m_levelsTuple);
                auto f = [&](auto&... args) { return std::tuple(args.begin()...); };

                return iterator{ *this, std::apply(f, a) };
            }

            inline iterator end() const noexcept
            {
                auto static a = std::apply(
                    [&](auto&... args)
                    { return std::tuple(args.iter_helper(this->m_i, this->m_pkm1)...); },
                    this->m_levelsTuple);

                auto f = [&](auto&... args) { return std::tuple(args.end()...); };
                return iterator{ *this, std::apply(f, a) };
            }
        };

        coiteration_helper coiter_helper(F f, std::tuple<Is...> i, PK pkm1, Levels&... levels)
        {
            return coiteration_helper{ f, i, pkm1, levels... };
        }
    };
}

#endif

#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>

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
        private:
            std::tuple<Is...> const m_i;
            PK const m_pkm1;
            std::tuple<Levels&...> const m_levelsTuple;
            F const m_comparisonHelper;

        public:
            explicit inline coiteration_helper(F f,
                                               std::tuple<Is...>& i,
                                               PK& pkm1,
                                               Levels&... levels)
                : m_levelsTuple(std::tie(levels...))
                , m_i(std::move(i))
                , m_pkm1(std::move(pkm1))
                , m_comparisonHelper(f)
            {
                static_assert(std::tuple_size_v<decltype(m_levelsTuple)> >= 2,
                              "Tuple size should be at least 2");
            }

            class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<Levels::template LevelCapabilities::template iteration_helper::
                               template iterator...>
                    iterators;
                IK min_ik;

            private:
                template <typename... T1, typename... T2>
                constexpr auto compareHelper(const std::tuple<T1...>& t1,
                                             const std::tuple<T2...>& t2)
                {
                    static_assert(sizeof...(T1) == sizeof...(T2));

                    return compare(t1, t2, std::make_index_sequence<sizeof...(T1)>{});
                }

                template <typename... T1, typename... T2, std::size_t... I>
                constexpr auto compare(const std::tuple<T1...>& t1,
                                       const std::tuple<T2...>& t2,
                                       std::index_sequence<I...>)
                {
                    return std::tuple{ std::get<I>(t1) == std::get<I>(t2)... };
                }

            public:
                using iterator_category = std::forward_iterator_tag;
                using reference = std::tuple<
                    IK,
                    std::tuple<std::optional<typename Levels::template BaseTraits::PK>...>>;

                explicit inline iterator(
                    coiteration_helper const& coiterHelper,
                    std::tuple<Levels::template LevelCapabilities::template iteration_helper::
                                   template iterator...> it)
                    : m_coiterHelper(coiterHelper)
                    , iterators(it)
                {
                    min_ik = static_cast<IK>(std::get<0>(*std::get<0>(iterators)));
                    std::apply(
                        [&](auto&&... args) {
                            ((min_ik = static_cast<IK>(std::min(min_ik, std::get<0>(*args)))), ...);
                        },
                        iterators);
                }

                auto get_PKs()
                {
                    return std::apply([&](auto&... args)
                                      { return std::make_tuple(locate(args)...); },
                                      this->iterators);
                }

                template <class iter>
                auto locate(iter i)
                {
                    return (std::get<0>(*i) == min_ik)
                               ? std::optional<std::tuple_element_t<1, decltype(*i)>>(
                                   std::get<1>(*i))
                               : std::nullopt;
                }

                template <class iter>
                void advance_iter(iter& i)
                {
                    if (static_cast<IK>(std::get<0>(*i)) == min_ik)
                    {
                        ++i;
                    }
                }

                reference operator*() noexcept
                {
                    auto PK_tuple = get_PKs();
                    return { min_ik, PK_tuple };
                }

                iterator operator++(int)
                {
                    iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                iterator& operator++()
                {
                    std::apply([&](auto&&... args) { ((advance_iter(args)), ...); }, iterators);

                    min_ik = static_cast<IK>(std::get<0>(*std::get<0>(iterators)));
                    std::apply(
                        [&](auto&&... args) {
                            ((min_ik = static_cast<IK>(std::min(min_ik, std::get<0>(*args)))), ...);
                        },
                        iterators);
                    return *this;
                }

                inline bool operator!=(const iterator& other)
                {
                    return !(*this == other);
                };

                inline bool operator==(const iterator& other)
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

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
            std::tuple<Is...> m_i;
            PK m_pkm1;
            std::tuple<Levels&...> m_levelsTuple;
            std::tuple<Levels::template LevelCapabilities::template iteration_helper...>
                iterHelpers;
            F m_comparisonHelper;

        public:
            explicit inline coiteration_helper(F f, std::tuple<Is...> i, PK pkm1, Levels&... levels)
                : m_levelsTuple(std::tie(levels...))
                , iterHelpers(std::apply([&](auto&... args)
                                         { return std::make_tuple(args.iter_helper(i, pkm1)...); },
                                         this->m_levelsTuple))
                , m_comparisonHelper(f)
            {
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
                template <size_t I = 0, typename... T1s, typename... T2s, typename... T3s>
                constexpr auto compare(std::tuple<T1s...> tup1,
                                       std::tuple<T2s...> tup2,
                                       std::tuple<T3s...> tup3)
                {
                    if constexpr (I == sizeof...(T1s))
                    {
                        return tup3;
                    }
                    else
                    {
                        return compare<I + 1>(
                            tup1,
                            tup2,
                            std::tuple_cat(
                                tup3, std::make_tuple(std::get<I>(tup1) != std::get<I>(tup2))));
                    }
                }

            public:
                using iterator_category = std::forward_iterator_tag;
                using key_type = IK;
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
                    std::apply([&](auto&&... args)
                               { ((min_ik = std::min(min_ik, std::get<0>(*args))), ...); },
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
                    if (std::get<0>(*i) == min_ik)
                    {
                        i = ++i;
                    }
                }

                reference operator*() noexcept
                {
                    std::apply([&](auto&&... args)
                               { ((min_ik = std::min(min_ik, std::get<0>(*args))), ...); },
                               iterators);
                    auto PK_tuple = get_PKs();
                    return { min_ik, PK_tuple };
                }

                iterator& operator++()
                {
                    std::apply([&](auto&... args) { ((advance_iter(args)), ...); }, iterators);
                    min_ik = static_cast<IK>(std::get<0>(*std::get<0>(iterators)));
                    std::apply([&](auto&&... args)
                               { ((min_ik = std::min(min_ik, std::get<0>(*args))), ...); },
                               iterators);
                    return *this;
                }

                inline bool operator!=(const iterator& other)
                {
                    return m_coiterHelper.m_comparisonHelper(
                        compare(iterators, other.iterators, std::make_tuple()));
                };

                inline bool operator==(const iterator& other)
                {
                    auto t = compare(iterators, other.iterators, std::make_tuple());
                    return m_coiterHelper.m_comparisonHelper(
                        std::apply([&](auto&... args) { return std::make_tuple(!args...); }, t));
                };
            };

            inline iterator begin() const noexcept
            {
                auto f = [](const auto&... args) { return std::tuple(args.begin()...); };
                return iterator{ *this, std::apply(f, this->iterHelpers) };
            }

            inline iterator end() const noexcept
            {
                auto f = [](const auto&... args) { return std::tuple(args.end()...); };
                return iterator{ *this, std::apply(f, this->iterHelpers) };
            }
        };

        coiteration_helper coiter_helper(F f, std::tuple<Is...> i, PK pkm1, Levels&... levels)
        {
            return coiteration_helper{ f, i, pkm1, levels... };
        }
    };
}
#endif

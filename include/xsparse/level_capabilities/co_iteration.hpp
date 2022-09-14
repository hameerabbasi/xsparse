#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>

namespace xsparse::level_capabilities
{
    template <class IK, class PK, class Levels, class Is>
    class Coiterate;

    template <class IK, class PK, class... Levels, class... Is>
    class Coiterate<IK, PK, std::tuple<Levels...>, std::tuple<Is...>>
    {
    public:
        class coiteration_helper
        {
        private:
            std::tuple<Is...> I;
            PK m_pkm1;
            std::tuple<Levels...> levelsTuple;
            std::tuple<Levels::template LevelCapabilities::template iteration_helper...>
                iterHelpers;

        public:
            explicit inline coiteration_helper(std::tuple<Is...> i,
                                               PK pkm1,
                                               Levels&... levels)
                : levelsTuple(std::make_tuple(levels...))
                , iterHelpers(std::apply([&](auto&... args)
                                         { return std::make_tuple(args.iter_helper(i, pkm1)...); },
                                         this->levelsTuple))
            {
            }

            class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<Levels::template LevelCapabilities::template iteration_helper::
                               template iterator...> iterators;
                IK min_ik;

            public:
                using iterator_category = std::forward_iterator_tag;
                using key_type = IK;
                using pointer = std::tuple<IK, std::tuple<std::optional<Levels::template PK>...>>*;
                using reference = std::tuple<IK, std::tuple<std::optional<Levels::template PK>...>>;

                explicit inline iterator(coiteration_helper const& coiterHelper,
                                         std::tuple<IterHelpers::template iterator...> it)
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
                    return std::apply([&](auto&... args) { return std::make_tuple(*locate(args)...); },
                               this->iterators);
                }

                template <class iter>
                auto locate(iter i)
                {
                    return (std::get<0>(*i) == min) ? std::optional<PK>(std::get<1>(*i))
                                                    : std::nullopt;
                }

                template <class iter>
                void advance_iter(iter& i)
                {
                    if (std::get<0>(*i) == min)
                    {
                        i = ++i;
                    }
                }

                reference operator*() noexcept
                {
                    std::apply([&](auto&&... args)
                               { ((min = std::min(min, std::get<0>(*args))), ...); },
                               iterators);
                    auto PK_tuple = get_PKs();
                    return { min, PK_tuple };
                }

                iterator& operator++()
                {
                    std::apply([&](auto&... args) { ((advance_iter(args)), ...); }, iterators);
                    min = static_cast<IK>(std::get<0>(*std::get<0>(iterators)));
                    std::apply([&](auto&&... args)
                               { ((min = std::min(min, std::get<0>(*args))), ...); },
                               iterators);
                    return *this;
                }

                friend bool operator==(const iterator& a, const iterator& b){
                    return a.iterators == b.iterators;
                };

                friend bool operator!=(const iterator& a, const iterator& b){
                    return a.iterators != b.iterators;
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

        coiteration_helper coiter_helper(std::tuple<Is...> i, PK pkm1, Levels... levels)
        {
            return coiteration_helper{ i, pkm1, levels... };
        }
    };

#endif

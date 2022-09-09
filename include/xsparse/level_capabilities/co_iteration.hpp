#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include <vector>
#include <tuple>

namespace xsparse::level_capabilities
{
    template <class IK, class PK, class Levels, class IterHelpers>
    class Coiterate;

    template <class IK, class PK, class... Levels, class... IterHelpers>
    class Coiterate<IK, PK, std::tuple<Levels...>, std::tuple<IterHelpers...>>
    {
    public:
        class coiteration_helper
        {
        private:
            std::tuple<Levels...> levelsTuple;
            std::tuple<IterHelpers...> iterHelpers;

        public:
            explicit inline coiteration_helper(std::tuple<IterHelpers...> iter_helpers,
                                               Levels... levels)
                : levelsTuple(std::make_tuple(levels...))
                , iterHelpers(iter_helpers)
            {
            }

            class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<IterHelpers::template iterator...> iterators;
                IK min;

            public:
                using iterator_category = std::forward_iterator_tag;
                using key_type = IK;
                using pointer = std::pair<IK, std::vector<std::optional<PK>>>*;
                using reference = std::tuple<IK, std::vector<std::optional<PK>>>;

                explicit inline iterator(coiteration_helper const& coiterHelper,
                                         std::tuple<IterHelpers::template iterator...> it)
                    : m_coiterHelper(coiterHelper)
                    , iterators(it)
                {
                    min = static_cast<IK>(std::get<0>(*std::get<0>(iterators)));
                    std::apply([&](auto&&... args)
                               { ((min = std::min(min, std::get<0>(*args))), ...); },
                               iterators);
                }

                auto get_PKs()
                {
                    std::vector<std::optional<PK>> vec;
                    std::apply([&](const auto&... args) { (vec.push_back(*locate(args)), ...); },
                               iterators);
                    return vec;
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

                /*friend bool operator==(const iterator& a, const iterator& b){

                };

                friend bool operator!=(const iterator& a, const iterator& b){

                };*/
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

        coiteration_helper coiter_helper(std::tuple<IterHelpers...> ih, Levels... levels)
        {
            return coiteration_helper{ ih, levels... };
        }
    };

#endif

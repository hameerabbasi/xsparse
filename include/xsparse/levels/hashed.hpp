#ifndef XSPARSE_LEVELS_HASHED_HPP
#define XSPARSE_LEVELS_HASHED_HPP

#include <tuple>
#include <utility>
#include <optional>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <xsparse/util/base_traits.hpp>
#include <xtl/xiterator_base.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits
                  = util::container_traits<std::vector, std::unordered_set, std::unordered_map>>
        class hashed;

        template <class... LowerLevels, class IK, class PK, class ContainerTraits>
        class hashed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits>
        {
            using BaseTraits
                = util::base_traits<hashed, std::tuple<LowerLevels...>, IK, PK, ContainerTraits>;
            using CrdContainer
                = ContainerTraits::template Vec<ContainerTraits::template Map<IK, PK>>;

        public:
            class iteration_helper
            {
                static_assert(std::is_nothrow_invocable_r_v<std::optional<typename BaseTraits::PK>,
                                                            decltype(&BaseTraits::Level::locate),
                                                            typename BaseTraits::Level&,
                                                            typename BaseTraits::PKM1,
                                                            typename BaseTraits::IK>);

            private:
                ContainerTraits::template Map<BaseTraits::IK, BaseTraits::PK>& m_map;

            public:
                class iterator;
                using value_type =
                    typename std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
                using difference_type = typename std::make_signed_t<typename BaseTraits::PK>;
                using pointer =
                    typename std::pair<typename BaseTraits::IK, typename BaseTraits::PK>*;
                using reference = std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
                using iterator_type = iterator;

                class iterator : public xtl::xbidirectional_iterator_base2<iteration_helper>
                {
                private:
                    using wrapped_iterator_type
                        = ContainerTraits::template Map<BaseTraits::IK,
                                                        BaseTraits::PK>::const_iterator;
                    wrapped_iterator_type wrapped_it;

                public:
                    explicit inline iterator(wrapped_iterator_type wrapped) noexcept
                        : wrapped_it(wrapped)
                    {
                    }

                    inline std::tuple<typename BaseTraits::IK, typename BaseTraits::PK> operator*()
                        const noexcept
                    {
                        return { wrapped_it->first, wrapped_it->second };
                    }

                    inline bool operator==(const iterator& other) const noexcept
                    {
                        return wrapped_it == other.wrapped_it;
                    }

                    inline iterator& operator++() noexcept
                    {
                        ++wrapped_it;
                        return *this;
                    }

                    inline iterator& operator--() noexcept
                    {
                        --wrapped_it;
                        return *this;
                    }
                };

                explicit inline iteration_helper(
                    ContainerTraits::template Map<BaseTraits::IK, BaseTraits::PK>& map) noexcept
                    : m_map(map)
                {
                }

                inline iterator_type begin() const noexcept
                {
                    return iterator_type{ m_map.begin() };
                }

                inline iterator_type end() const noexcept
                {
                    return iterator_type{ m_map.end() };
                }
            };

            iteration_helper iter_helper(typename BaseTraits::PKM1 pkm1)
            {
                return iteration_helper{ this->m_crd[pkm1] };
            }

            hashed(IK size)
                : m_size(std::move(size))
                , m_crd()
            {
            }

            hashed(IK size, CrdContainer const& crd)
                : m_size(std::move(size))
                , m_crd(crd)
            {
            }

            hashed(IK size, CrdContainer&& crd)
                : m_size(std::move(size))
                , m_crd(crd)
            {
            }

            inline auto locate(typename BaseTraits::PKM1 pkm1, IK ik) const noexcept
            {
                auto it = m_crd[pkm1].find(ik);
                return it != m_crd[pkm1].end() ? std::optional<PK>(it->second) : std::nullopt;
            }

            inline void insert_init(typename BaseTraits::IK szkm1) noexcept
            {
                m_crd.resize(szkm1);
            }

            inline void insert_coord(typename BaseTraits::PKM1 pkm1, PK pk, IK ik) noexcept
            {
                m_crd[pkm1][ik] = pk;
            }

        private:
            IK m_size;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels, class IK, class PK, class ContainerTraits>
    struct util::coordinate_position_trait<
        levels::hashed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

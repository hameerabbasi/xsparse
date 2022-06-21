#ifndef XSPARSE_LEVELS_HASHED_HPP
#define XSPARSE_LEVELS_HASHED_HPP

#include <tuple>
#include <utility>
#include <optional>
#include <unordered_map>

#include <xsparse/util/base_traits.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels, class IK, class PK, class CrdContainer>
        class hashed;

        template <class... LowerLevels, class IK, class PK, class CrdContainer>
        class hashed<std::tuple<LowerLevels...>, IK, PK, CrdContainer>
        {
            using BaseTraits
                = util::base_traits<hashed, std::tuple<LowerLevels...>, IK, PK, CrdContainer>;

        public:
            class iteration_helper
            {
                static_assert(std::is_nothrow_invocable_r_v<std::optional<typename BaseTraits::PK>,
                                                            decltype(&BaseTraits::Level::locate),
                                                            typename BaseTraits::Level&,
                                                            typename BaseTraits::PKM1,
                                                            typename BaseTraits::IK>);

            private:
                typename BaseTraits::Level const& m_level;
                typename BaseTraits::PKM1 const m_pkm1;

            public:
                class iterator;
                using value_type = typename BaseTraits::PK;
                using difference_type = typename std::make_signed_t<typename BaseTraits::PK>;
                using key_type = typename BaseTraits::IK;
                using pointer = typename BaseTraits::PK*;
                using reference = std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
                using iterator_type = std::unordered_map<typename BaseTraits::IK,
                                                         typename BaseTraits::PK>::const_iterator;

                explicit inline iteration_helper(typename BaseTraits::Level const& level,
                                                 typename BaseTraits::PKM1 const pkm1) noexcept
                    : m_level(level)
                    , m_pkm1(pkm1)
                {
                }

                inline iterator_type begin() const noexcept
                {
                    return m_level.m_crd[m_pkm1].cbegin();
                }

                inline iterator_type end() const noexcept
                {
                    return m_level.m_crd[m_pkm1].cend();
                }
            };

            iteration_helper iter_helper(typename BaseTraits::PKM1 pkm1)
            {
                return iteration_helper{ *static_cast<typename BaseTraits::Level*>(this), pkm1 };
            }

            hashed(IK size)
                : m_crd_size(std::move(size))
                , m_crd()
            {
            }

            hashed(IK size, CrdContainer const& crd)
                : m_crd_size(std::move(size))
                , m_crd(crd)
            {
            }

            hashed(IK size, CrdContainer&& crd)
                : m_crd_size(std::move(size))
                , m_crd(crd)
            {
            }

            inline auto locate(typename BaseTraits::PKM1 pkm1, IK ik) const noexcept
            {
                auto it = m_crd[pkm1].find(ik);
                return it != m_crd[pkm1].end() ? std::optional<PK>(it->second) : std::nullopt;
            }

            inline void insert_init(typename BaseTraits::IK szkm1) const noexcept
            {
                m_crd.resize(szkm1);
            }

            inline void insert_coord(typename BaseTraits::PKM1 pkm1, PK pk, IK ik) const noexcept
            {
                m_crd[pkm1][ik] = pk;
            }

            inline void insert_edges(typename BaseTraits::PKM1 pkm1) const noexcept
            {
                m_crd_size += m_crd[pkm1].size();
            }

        private:
            PK m_crd_size;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels, class IK, class PK, class CrdContainer>
    struct util::coordinate_position_trait<
        levels::hashed<std::tuple<LowerLevels...>, IK, PK, CrdContainer>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

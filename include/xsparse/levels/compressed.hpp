#ifndef XSPARSE_LEVELS_COMPRESSED_HPP
#define XSPARSE_LEVELS_COMPRESSED_HPP

#include <tuple>
#include <utility>

#include <xsparse/level_capabilities/coordinate_iterate.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels, class IK, class PK, class PosContainer, class CrdContainer>
        class compressed;

        template <class... LowerLevels, class IK, class PK, class PosContainer, class CrdContainer>
        class compressed<std::tuple<LowerLevels...>, IK, PK, PosContainer, CrdContainer>
            : public level_capabilities::coordinate_position_iterate<compressed,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     PosContainer,
                                                                     CrdContainer>

        {
            using BaseTraits = util::base_traits<compressed,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 PosContainer,
                                                 CrdContainer>;

        public:
            compressed(IK size)
                : m_size(std::move(size))
                , m_pos()
                , m_crd()
            {
            }

            compressed(IK size, PosContainer const& pos, CrdContainer const& crd)
                : m_size(std::move(size))
                , m_pos(pos)
                , m_crd(crd)
            {
            }

            compressed(IK size, PosContainer&& pos, CrdContainer&& crd)
                : m_size(std::move(size))
                , m_pos(pos)
                , m_crd(crd)
            {
            }

            inline std::pair<PK, PK> pos_bounds(typename BaseTraits::PKM1 pkm1) const noexcept
            {
                return { m_pos[pkm1], m_pos[static_cast<typename BaseTraits::PKM1>(pkm1 + 1)] };
            }

            inline IK pos_access(PK pk, [[maybe_unused]] typename BaseTraits::I i) const noexcept
            {
                return m_crd[pk];
            }

        private:
            IK m_size;
            PosContainer m_pos;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels, class IK, class PK, class PosContainer, class CrdContainer>
    struct util::coordinate_position_trait<
        levels::compressed<std::tuple<LowerLevels...>, IK, PK, PosContainer, CrdContainer>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

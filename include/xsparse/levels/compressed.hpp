#ifndef XSPARSE_LEVELS_COMPRESSED_HPP
#define XSPARSE_LEVELS_COMPRESSED_HPP

#include <tuple>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <xsparse/level_capabilities/coordinate_iterate.hpp>
#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits
                  = util::container_traits<std::vector, std::unordered_set, std::unordered_map>,
                  class LevelProperties = level_properties<true, true, true, false, true>>
        class compressed;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class LevelProperties>
        class compressed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, LevelProperties>
            : public level_capabilities::coordinate_position_iterate<compressed,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     ContainerTraits,
                                                                     LevelProperties>

        {
            static_assert(!LevelProperties::is_branchless);
            static_assert(LevelProperties::is_compact);
            using PosContainer = typename ContainerTraits::template Vec<PK>;
            using CrdContainer = typename ContainerTraits::template Vec<IK>;

        public:
            using BaseTraits = util::base_traits<compressed,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 LevelProperties>;
            using LevelCapabilities
                = level_capabilities::coordinate_position_iterate<compressed,
                                                                  std::tuple<LowerLevels...>,
                                                                  IK,
                                                                  PK,
                                                                  ContainerTraits,
                                                                  LevelProperties>;

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

            inline void append_init(typename BaseTraits::IK szkm1) noexcept
            {
                m_pos.resize(szkm1 + 1);
            }

            inline void append_edges(typename BaseTraits::PKM1 pkm1,
                                     typename BaseTraits::PK pk_begin,
                                     typename BaseTraits::PK pk_end) noexcept
            {
                m_pos[pkm1 + 1] = pk_end - pk_begin;
            }

            inline void append_coord(typename BaseTraits::IK ik) noexcept
            {
                m_crd.push_back(ik);
            }

            inline void append_finalize(typename BaseTraits::IK szkm1) noexcept
            {
                typename BaseTraits::PK cumsum = m_pos[0];
                for (typename BaseTraits::PK pkm1 = 1; pkm1 <= szkm1; ++pkm1)
                {
                    cumsum += m_pos[pkm1];
                    m_pos[pkm1] = cumsum;
                }
            }

        private:
            IK m_size;
            PosContainer m_pos;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels,
              class IK,
              class PK,
              class ContainerTraits,
              class LevelProperties>
    struct util::coordinate_position_trait<
        levels::compressed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

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
                  class _LevelProperties = level_properties<true, true, true, false, true>>
        class compressed;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class _LevelProperties>
        class compressed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>
            : public level_capabilities::coordinate_position_iterate<compressed,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     ContainerTraits,
                                                                     _LevelProperties>

        {
            static_assert(!_LevelProperties::is_branchless);
            static_assert(_LevelProperties::is_compact);
            using PosContainer = typename ContainerTraits::template Vec<PK>;
            using CrdContainer = typename ContainerTraits::template Vec<IK>;

        public:
            using BaseTraits = util::base_traits<compressed,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 _LevelProperties>;
            using LevelCapabilities
                = level_capabilities::coordinate_position_iterate<compressed,
                                                                  std::tuple<LowerLevels...>,
                                                                  IK,
                                                                  PK,
                                                                  ContainerTraits,
                                                                  _LevelProperties>;
            using LevelProperties = _LevelProperties;

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
            /**
             * @brief Initialize the compressed level before appending data to it.
             * 
             * @param szkm1 - the size of the previous level.
             */
            {
                m_pos.resize(szkm1 + 1);
            }

            inline void append_edges(typename BaseTraits::PKM1 pkm1,
                                     typename BaseTraits::PK pk_begin,
                                     typename BaseTraits::PK pk_end) noexcept
            /**
             * @brief Append edge information to the compressed level.
             * 
             * @param pkm1 - the previous level position where the edge information
             * is to be stored.
             * @param pk_begin - the beginning position of the edges in previous
             * level.
             * @param pk_end - the ending position of the edges in previous level.
             * 
             */
            {
                m_pos[pkm1 + 1] = pk_end - pk_begin;
            }

            inline void append_coord(typename BaseTraits::IK ik) noexcept
            /**
             * @brief Append coordinate information to the compressed level.
             * 
             * @param ik - the coordinate information to be appended.
             */
            {
                m_crd.push_back(ik);
            }

            inline void append_finalize(typename BaseTraits::IK szkm1) noexcept
            /**
             * @brief Finalize the compressed level after appending data to it.
             * 
             * @param szkm1 - the size of the previous level.
             */
            {
                typename BaseTraits::PK cumsum = m_pos[0];
                for (typename BaseTraits::PK pkm1 = 1; pkm1 <= szkm1; ++pkm1)
                {
                    cumsum += m_pos[pkm1];
                    m_pos[pkm1] = cumsum;
                }
            }

            inline IK size() const noexcept
            {
                return m_size;
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
              class _LevelProperties>
    struct util::coordinate_position_trait<
        levels::compressed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

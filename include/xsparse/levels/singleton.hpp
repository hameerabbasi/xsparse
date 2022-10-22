#ifndef XSPARSE_LEVELS_SINGLETON_HPP
#define XSPARSE_LEVELS_SINGLETON_HPP

#include <tuple>
#include <utility>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits
                  = util::container_traits<std::vector, std::unordered_set, std::unordered_map>,
                  class LevelProperties = level_properties<true, true, true, true, true>>
        class singleton;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class LevelProperties>
        class singleton<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, LevelProperties>
            : public level_capabilities::coordinate_position_iterate<singleton,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     ContainerTraits,
                                                                     LevelProperties>

        {
            static_assert(LevelProperties::is_branchless);
            static_assert(LevelProperties::is_compact);
            using CrdContainer = typename ContainerTraits::template Vec<PK>;

        public:
            using BaseTraits = util::base_traits<singleton,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 LevelProperties>;
            using LevelCapabilities
                = level_capabilities::coordinate_position_iterate<singleton,
                                                                  std::tuple<LowerLevels...>,
                                                                  IK,
                                                                  PK,
                                                                  ContainerTraits,
                                                                  LevelProperties>;

        public:
            singleton(IK size)
                : m_size(std::move(size))
                , m_crd()
            {
            }

            singleton(IK size, CrdContainer const& crd)
                : m_size(std::move(size))
                , m_crd(crd)
            {
            }

            singleton(IK size, CrdContainer&& crd)
                : m_size(std::move(size))
                , m_crd(crd)
            {
            }

            inline std::pair<PK, PK> pos_bounds(typename BaseTraits::PKM1 pkm1) const noexcept
            {
                return { static_cast<PK>(pkm1), static_cast<PK>(pkm1 + 1) };
            }

            inline IK pos_access(PK pk, [[maybe_unused]] typename BaseTraits::I i) const noexcept
            {
                return m_crd[pk];
            }

            inline void append_coord(typename BaseTraits::IK ik) noexcept
            {
                m_crd.push_back(ik);
            }

            inline IK size() const noexcept
            {
                return m_size;
            }

        private:
            IK m_size;

        private:
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels,
              class IK,
              class PK,
              class ContainerTraits,
              class LevelProperties>
    struct util::coordinate_position_trait<
        levels::singleton<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

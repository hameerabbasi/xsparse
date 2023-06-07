#ifndef XSPARSE_LEVELS_HASHED_HPP
#define XSPARSE_LEVELS_HASHED_HPP

#include <tuple>
#include <utility>
#include <optional>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <xsparse/util/base_traits.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>
#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>
#include <xtl/xiterator_base.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits
                  = util::container_traits<std::vector, std::unordered_set, std::unordered_map>,
                  class _LevelProperties = level_properties<true, false, true, false, false>>
        class hashed;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class _LevelProperties>
        class hashed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>
            : public level_capabilities::coordinate_value_iterate<hashed,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     _LevelProperties>
        {
            static_assert(!_LevelProperties::is_ordered);
            static_assert(!_LevelProperties::is_branchless);
            static_assert(!_LevelProperties::is_compact);
            using CrdContainer = typename ContainerTraits::template Vec<
                typename ContainerTraits::template Map<IK, PK>>;

        public:
            using LevelCapabilities
                = level_capabilities::locate_position_iterate<hashed,
                                                                 std::tuple<LowerLevels...>,
                                                                 IK,
                                                                 PK,
                                                                 ContainerTraits,
                                                                 _LevelProperties>;
            using BaseTraits = util::base_traits<hashed,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 _LevelProperties>;

            using LevelProperties = _LevelProperties;

        public:
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

            inline IK size() const noexcept
            {
                return m_size;
            }

        private:
            IK m_size;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels,
              class IK,
              class PK,
              class ContainerTraits,
              class _LevelProperties>
    struct util::coordinate_position_trait<
        levels::hashed<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

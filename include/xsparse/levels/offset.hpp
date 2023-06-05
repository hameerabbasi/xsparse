#ifndef XSPARSE_LEVELS_OFFSET_HPP
#define XSPARSE_LEVELS_OFFSET_HPP

#include <tuple>
#include <utility>
#include <xsparse/util/base_traits.hpp>
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
                  class _LevelProperties = level_properties<false, true, true, true, false>>
        class offset;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class _LevelProperties>
        class offset<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>
            : public level_capabilities::coordinate_position_iterate<offset,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     ContainerTraits,
                                                                     _LevelProperties>

        {
            static_assert(!_LevelProperties::is_full);
            static_assert(_LevelProperties::is_branchless);
            static_assert(!_LevelProperties::is_compact);
            using OffsetContainer = typename ContainerTraits::template Vec<PK>;

        public:
            using BaseTraits = util::base_traits<offset,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 _LevelProperties>;
            using LevelCapabilities
                = level_capabilities::coordinate_position_iterate<offset,
                                                                  std::tuple<LowerLevels...>,
                                                                  IK,
                                                                  PK,
                                                                  ContainerTraits,
                                                                  _LevelProperties>;
            using LevelProperties = _LevelProperties;

        public:
            offset(IK size)
                : m_size(std::move(size))
                , m_offset()
            {
            }

            offset(IK size, OffsetContainer const& offset)
                : m_size(std::move(size))
                , m_offset(offset)
            {
            }

            offset(IK size, OffsetContainer&& offset)
                : m_size(std::move(size))
                , m_offset(offset)
            {
            }

            inline std::pair<PK, PK> pos_bounds(typename BaseTraits::PKM1 pkm1) const noexcept
            {
                return { static_cast<PK>(pkm1), static_cast<PK>(pkm1 + 1) };
            }

            inline IK pos_access([[maybe_unused]] PK pk, typename BaseTraits::I i) const noexcept
            {
                static_assert(std::tuple_size_v<decltype(i)> >= 2,
                              "Tuple size should at least be 2");
                return static_cast<IK>(std::get<0>(i) + m_offset[std::get<1>(i)]);
            }

            inline IK size() const noexcept
            {
                return m_size;
            }

        private:
            IK m_size;
            OffsetContainer m_offset;
        };
    }  // namespace levels

    template <class... LowerLevels,
              class IK,
              class PK,
              class ContainerTraits,
              class _LevelProperties>
    struct util::coordinate_position_trait<
        levels::offset<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

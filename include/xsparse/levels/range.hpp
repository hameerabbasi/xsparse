#ifndef XSPARSE_LEVELS_RANGE_HPP
#define XSPARSE_LEVELS_RANGE_HPP

#include <utility>
#include <xsparse/util/base_traits.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>
#include <tuple>

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
                  class _LevelProperties = level_properties<false, true, true, false, false>>
        class range;

        template <class... LowerLevels,
                  class IK,
                  class PK,
                  class ContainerTraits,
                  class _LevelProperties>
        class range<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>
            : public level_capabilities::coordinate_value_iterate<range,
                                                                  std::tuple<LowerLevels...>,
                                                                  IK,
                                                                  PK,
                                                                  ContainerTraits,
                                                                  _LevelProperties>
        {
            static_assert(!_LevelProperties::is_full);
            static_assert(!_LevelProperties::is_branchless);
            static_assert(!_LevelProperties::is_compact);
            using OffsetContainer = typename ContainerTraits::template Vec<PK>;

        public:
            using BaseTraits = util::base_traits<range,
                                                 std::tuple<LowerLevels...>,
                                                 IK,
                                                 PK,
                                                 ContainerTraits,
                                                 _LevelProperties>;
            using LevelCapabilities
                = level_capabilities::coordinate_value_iterate<range,
                                                               std::tuple<LowerLevels...>,
                                                               IK,
                                                               PK,
                                                               ContainerTraits,
                                                               _LevelProperties>;
            using LevelProperties = _LevelProperties;

        public:
            range(IK size_N, IK size_M)
                : m_size_N(std::move(size_N))
                , m_size_M(std::move(size_M))
                , m_offset()
            {
            }

            range(IK size_N, IK size_M, OffsetContainer const& offset)
                : m_size_N(std::move(size_N))
                , m_size_M(std::move(size_M))
                , m_offset(offset)
            {
            }

            range(IK size_N, IK size_M, OffsetContainer&& offset)
                : m_size_N(std::move(size_N))
                , m_size_M(std::move(size_M))
                , m_offset(offset)
            {
            }

            inline std::pair<IK, IK> coord_bounds(typename BaseTraits::I i) const noexcept
            {
                static_assert(std::tuple_size_v<decltype(i)> >= 1,
                              "Tuple size should be at least 1");
                return { static_cast<IK>(std::max(0, -m_offset[std::get<0>(i)])),
                         static_cast<IK>(std::min(m_size_N, m_size_M - m_offset[std::get<0>(i)])) };
            }

            inline std::optional<PK> coord_access(typename BaseTraits::PKM1 pkm1,
                                                  [[maybe_unused]] typename BaseTraits::I i,
                                                  IK ik) const noexcept
            {
                return std::optional(static_cast<PK>(pkm1 * m_size_N + ik));
            }

        private:
            IK m_size_N, m_size_M;
            OffsetContainer m_offset;
        };
    }

    template <class... LowerLevels,
              class IK,
              class PK,
              class ContainerTraits,
              class _LevelProperties>
    struct util::coordinate_position_trait<
        levels::range<std::tuple<LowerLevels...>, IK, PK, ContainerTraits, _LevelProperties>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}

#endif  // XSPARSE_LEVELS_RANGE_HPP

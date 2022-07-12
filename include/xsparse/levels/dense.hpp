#ifndef XSPARSE_LEVELS_DENSE_HPP
#define XSPARSE_LEVELS_DENSE_HPP

#include <utility>
#include <xsparse/util/base_traits.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels, class IK, class PK>
        class dense;

        template <class... LowerLevels, class IK, class PK>
        class dense<std::tuple<LowerLevels...>, IK, PK>
            : public level_capabilities::
                  coordinate_value_iterate<dense, std::tuple<LowerLevels...>, IK, PK>

        {
            using BaseTraits = util::base_traits<dense, std::tuple<LowerLevels...>, IK, PK>;

        public:
            dense(IK size)
                : m_size(std::move(size))
            {
            }

            inline std::pair<IK, IK> coord_bounds([[maybe_unused]]
                                                  typename BaseTraits::I i) const noexcept
            {
                return { static_cast<IK>(0), m_size };
            }

            inline std::optional<PK> coord_access(typename BaseTraits::PKM1 pkm1,
                                                  [[maybe_unused]] typename BaseTraits::I i,
                                                  IK ik) const noexcept
            {
                return std::optional(static_cast<PK>(pkm1 * m_size + ik));
            }

            inline BaseTraits::IK size(typename BaseTraits::IK szkm1) const noexcept
            {
                return szkm1 * m_size;
            }

        private:
            IK m_size;
        };
    }

    template <class... LowerLevels, class IK, class PK>
    struct util::coordinate_position_trait<levels::dense<std::tuple<LowerLevels...>, IK, PK>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}

#endif  // XSPARSE_LEVELS_DENSE_HPP

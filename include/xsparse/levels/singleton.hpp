#ifndef XSPARSE_LEVELS_SINGLETON_HPP
#define XSPARSE_LEVELS_SINGLETON_HPP

#include <tuple>
#include <utility>

#include <xsparse/level_capabilities/coordinate_iterate.hpp>

namespace xsparse
{
    namespace levels
    {
        template <class LowerLevels, class IK, class PK, class CrdContainer>
        class singleton;

        template <class... LowerLevels, class IK, class PK, class CrdContainer>
        class singleton<std::tuple<LowerLevels...>, IK, PK, CrdContainer>
            : public level_capabilities::coordinate_position_iterate<singleton,
                                                                     std::tuple<LowerLevels...>,
                                                                     IK,
                                                                     PK,
                                                                     CrdContainer>

        {
            using BaseTraits
                = util::base_traits<singleton, std::tuple<LowerLevels...>, IK, PK, CrdContainer>;

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

        private:
            IK m_size;
            CrdContainer m_crd;
        };
    }  // namespace levels

    template <class... LowerLevels, class IK, class PK, class CrdContainer>
    struct util::coordinate_position_trait<
        levels::singleton<std::tuple<LowerLevels...>, IK, PK, CrdContainer>>
    {
        using Coordinate = IK;
        using Position = PK;
    };
}  // namespace xsparse


#endif

#ifndef XSPARSE_MERGE_LATTICE_HPP
#define XSPARSE_MERGE_LATTICE_HPP

#include <tuple>

#include <xsparse/tensor.hpp>

namespace xsparse
{
    template <class F, std::size_t OutDim, class Tensor, class Is>
    class MergeLattice;

    template <class F, std::size_t OutDim, class... Tensor, class... Is>
    class MergeLattice<F, OutDim, std::tuple<Tensor...>, std::tuple<std::vector<Is>...>>
    {
        private:
            OutDim const m_outDim;
            std::tuple<Tensor&...> const m_tensors;
            std::tuple<std::vector<Is>&...> const m_is;

        public:
            explicit inline MergeLattice(OutDim outDim, Tensor&... tensors, std::vector<Is>&... is)
            : m_outDim(outDim), m_tensors(std::tie(tensors...)), m_is(std::tie(is...))
            {
            }

            inline constexpr auto get_merge_points() const noexcept
            {

            }

            inline constexpr auto merge_iterators() const noexcept
            {
                
            }
    };
}

#endif
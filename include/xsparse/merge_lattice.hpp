#ifndef XSPARSE_MERGE_LATTICE_HPP
#define XSPARSE_MERGE_LATTICE_HPP

#include <tuple>

#include <xsparse/tensor.hpp>

namespace xsparse
{
    template <class F, typename OutDim, class Tensor, class Is>
    class MergeLattice;

    template <class F, typename OutDim, class... Tensor, class... Is>
    class MergeLattice<F, OutDim, std::tuple<Tensor...>, std::tuple<std::vector<Is>...>>
    {
    private:
        OutDim const m_outDim;
        std::tuple<Tensor&...> const m_tensors;
        std::tuple<std::vector<Is>&...> const m_is;
        F const m_comparisonHelper;

    public:
        explicit inline MergeLattice(F f, OutDim outDim, Tensor&... tensors, std::tuple<std::vector<Is>>&... is)
            : m_outDim(outDim)
            , m_tensors(std::tie(tensors...))
            , m_is(std::tie(is...))
            , m_comparisonHelper(f)
        {
            static_assert(sizeof...(Tensor) == sizeof...(Is));

            // TODO: check that all levels of the tensor has the same dimensions as the size of each vector of indices
            // TODO: Each vector of input indices (is) should be strictly increasing
        }

        inline constexpr auto get_merge_points() const noexcept
        {
        }

        // TODO: What is the output format of the merge lattice?
        inline constexpr auto merge_iterators() const noexcept
        {
        }
    };
}

#endif
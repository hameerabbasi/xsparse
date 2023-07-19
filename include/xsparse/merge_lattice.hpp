#ifndef XSPARSE_MERGE_LATTICE_HPP
#define XSPARSE_MERGE_LATTICE_HPP

#include <tuple>

#include <xsparse/tensor.hpp>

namespace xsparse
{
    template <class F, class Tensor>
    class MergeLattice;

    template <class F, class... Tensor>
    class MergeLattice<F, std::tuple<Tensor...>, std::tuple<std::vector<std::size_t>...>>
    {
    private:
        // OutDim const m_outDim;
        std::tuple<Tensor&...> const m_tensors;
        std::tuple<std::vector<Is>&...> const m_is;
        // F const m_comparisonHelper;

    public:
        explicit inline MergeLattice(
            // F f, OutDim outDim, 
            // is -> make a pair of std::tuple
            // tuple of pairs of Tensors and vectors<size_t>
            std::tuple<Tensor...>& tensors, std::tuple<std::vector<std::size_t...>>& is)
            : 
            // m_outDim(outDim)
            , m_tensors(std::tie(tensors...))
            , m_is(std::tie(is...))
            // , m_comparisonHelper(f)
        {
            static_assert(sizeof...(Tensor) == sizeof...(Is));

            // TODO: check that all levels of the tensor has the same dimensions as the size of each vector of indices
            // TODO: Each vector of input indices (is) should be strictly increasing

            // TODO: runtime check that the shapes of the tensors match e.g. (i, j) of A_ij and B_ij
            // and the i of D_i.
        }

    // TODO: we also need to define an iterator in here that will actually iterate over the merge lattice.
    // begin() and end() would define the beginning and end of the iterator
    // begin, end and != would essentially be same as coiterate, but in between
    // e.g. advance would be different
    // dereference would return a pair which would have two elements for A,
    // (<index into the tensor (e.g. i,j,k tuple values)>, <tuple of of each tensor value at those index>)
    // the type of the tensor values is able to be gotten from Tensor::ContainerType
    // Ex: pair((i,j), (<ContainerType::value> A[ij], B[ij], D[i])) for the case we are discussing
    // advance would change because you need 
    };
}

#endif
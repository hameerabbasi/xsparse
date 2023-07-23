#ifndef XSPARSE_MERGE_LATTICE_HPP
#define XSPARSE_MERGE_LATTICE_HPP

#include <tuple>

#include <xsparse/tensor.hpp>
#include <xsparse/level_capabilities/co_iteration.hpp>

namespace xsparse
{
    template <class F, class Tensor>
    class MergeLattice;

    template <class F, class... Tensor>
    class MergeLattice<F,  std::tuple<Tensor...>> //std::pair<Tensor..., std::vector<std::size_t>>>
    {
    private:
        std::tuple<Tensor&...> const m_tensors;
        std::tuple<std::vector<std::size_t>> const m_is;

        // std::tuple<std::pair<Tensor, std::vector<int>>...> const m_tensors;
    public:
        explicit inline MergeLattice(
            // is -> make a pair of std::tuple
            // tuple of pairs of Tensors and vectors<size_t>
            // std::pair<std::tuple<Tensors...>, std::vector<std::size_t>>& tensor_and_indices)
            // std::tuple<std::pair<Tensor, std::vector<int>>...>& tensor_and_indices)
            Tensor&... tensors, std::tuple<std::vector<std::size_t>>& is)
            : m_tensors(std::tie(tensors...))
            , m_is(is)
            // : m_tensors(tensor_and_indices)
        {
            static_assert(sizeof...(Tensor) == sizeof(is));

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
    public:
        class iterator
        {
        private:
            // coiteration_helper const& m_coiterHelper;
            // std::tuple<typename Levels::iteration_helper::iterator...> iterators;
            // IK min_ik;
            // std::tuple<typename Levels::iteration_helper...> m_iterHelpers;

        public:
            using iterator_category = std::forward_iterator_tag;
            using reference = typename std::
                tuple<IK, std::tuple<std::optional<typename Levels::BaseTraits::PK>...>>;

            explicit inline iterator() noexcept
            {
            }

            inline reference operator*() const noexcept
            {
                // auto PK_tuple = get_PKs();
                // return { min_ik, PK_tuple };
            }

            inline iterator operator++(int) const noexcept
            {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            inline iterator& operator++() noexcept
            {
                // std::apply([&](auto&... args) { ((advance_iter(args)), ...); }, iterators);
                // min_helper();
                // return *this;
            }

            inline bool operator!=(iterator const& other) const noexcept
            {
                // return !m_coiterHelper.m_coiterate.m_comparisonHelper(
                //     compareHelper(iterators, other.iterators));
            };

            inline bool operator==(iterator const& other) const noexcept
            {
                // return !(*this != other);
            };
        };
        inline iterator begin() const noexcept
        /**
         * @brief Beginning of each tensor's iterator.
         * 
         */
        {
            return iterator{ *this,
                                std::apply([&](auto&... args)
                                        { return std::tuple(args.begin()...); },
                                        this->m_iterHelpers) };
        }

        inline iterator end() const noexcept
        /**
         * @brief End of each tensor's iterator.
         * 
         */
        {
            // return iterator{ *this,
            //                     std::apply([&](auto&... args)
            //                             { return std::tuple(args.end()...); },
            //                             this->m_iterHelpers) };
        }
    };
}

#endif
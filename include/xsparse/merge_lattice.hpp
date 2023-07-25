#ifndef XSPARSE_MERGE_LATTICE_HPP
#define XSPARSE_MERGE_LATTICE_HPP

#include <tuple>

#include <xsparse/tensor.hpp>
#include <xsparse/level_capabilities/co_iteration.hpp>

// utility method for template specialization of the function F
// Base case for recursion (no indices to set)
template <std::size_t... Indices, typename... Args>
auto
callF(std::index_sequence<Indices...>, std::tuple<Args...> args)
{
    // For demonstration purposes, let's print the values in the tuple
    std::cout << "Received tuple: ";
    std::apply([](const auto&... args) { ((std::cout << args << ' '), ...); }, args);
    std::cout << std::endl;

    return F<std::get<Indices>(args)...>::value;
}

// Recursive case for setting the first index and continuing with the rest
template <std::size_t Index, std::size_t... Indices, typename... Args>
auto
callF(std::index_sequence<Index, Indices...>, std::tuple<Args...> args)
{
    std::get<Index>(args) = true;
    return callF(std::index_sequence<Indices...>(), args);
}

// Helper function to call the recursive version explicitly
template <std::size_t... Indices, typename... Args>
auto
SetIndicesToTrue(std::tuple<Args...> args)
{
    return callF(std::index_sequence<Indices...>(), args);
}

namespace xsparse
{
    template <class F, typename DataType, typename Indices, class Tensors>
    class MergeLattice;

    template <class F, typename DataType, typename... Indices, class... Tensors>
    class MergeLattice<F, DataType, std::tuple<Indices...>, std::tuple<Tensors...>>
    {
    private:
        std::tuple<Tensors&...> const m_tensors;

    public:
        explicit inline MergeLattice(Tensors&... tensors)
            : m_tensors(std::tie(tensors...))
        {
            // https://godbolt.org/z/1YKoKndMa
            static_assert(sizeof...(Tensors) == sizeof...(Indices));

            // TODO: check that all levels of the tensor has the same dimensions as the size of each
            // vector of indices
            // TODO: Each vector of input indices (is) should be strictly increasing

            // TODO: runtime check that the shapes of the tensors match e.g. (i, j) of A_ij and B_ij
            // and the i of D_i.
        }

        // TODO: we also need to define an iterator in here that will actually iterate over the
        // merge lattice. begin() and end() would define the beginning and end of the iterator
        // begin, end and != would essentially be same as coiterate, but in between
        // e.g. advance would be different
        // dereference would return a pair which would have two elements for A,
        // (<index into the tensor (e.g. i,j,k tuple values)>, <tuple of of each tensor value at
        // those index>) the type of the tensor values is able to be gotten from
        // Tensors::ContainerType Ex: pair((i,j), (<ContainerType::value> A[ij], B[ij], D[i])) for
        // the case we are discussing advance would change because you need
    public:
        class iterator
        {
        private:
            // a tuple of coiterators defined for each index level
            std::tuple<xsparse::level_capabilities::Coiterate...> m_coiters;

            // number of coiterators is the same as the number of indices
            static constexpr auto n_coiterators = get_max_index<Indices...>();

            constexpr auto get_max_index() const noexcept
            /**
             * @brief Get the maximum index from tuple of vectors of indices.
             *
             */
            {
            }

            template <std::size_t I, class... Levels>
            constexpr auto init_coiterator_for_levels() noexcept
            {
                if constexpr (I < n_coiterators)
                {
                    return std::tuple_cat(
                        std::make_tuple(
                            xsparse::level_capabilities::Coiterate(std::get<I>(m_tensors))
                        ), init_coiterator_for_levels<I + 1>()
                    );
                }

                // compute which indices are not involved, so they are always set to true
                // constexpr auto default_true_indices;

                // re-define the template function F to take in a tuple of bools where
                // the indices in `default_true_indices` are always set to true

                // define the coiterator for index I
                xsparse::level_capabilities::Coiterate<F,
                                                       uintptr_t,
                                                       uintptr_t,
                                                       std::tuple<decltype(Levels)...>,
                                                       std::tuple<>>
                coiter(Levels...);
                return coiter;
            }

            constexpr auto init_coiterators() noexcept
            {
                // a tuple of coiterators
                constexpr auto coiterators = init_coiterator_for_levels<0>();
                return coiterators;
            }

        public:
            using iterator_category = std::forward_iterator_tag;
            using reference = DataType;

            explicit inline iterator() noexcept
                : m_coiters(init_coiterators())
            {
            }

            inline reference operator*() const noexcept
            {
                // dereference each coiterator and return a tuple of the values
                // for coiter in m_coiters : auto [ik, pk] = coiter*;


                // auto PK_tuple = get_PKs();
                return { min_ik, PK_tuple };
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
                /**
                 * @brief Check if two merge lattice iterators are not equal.
                 *
                 * @details Two merge lattice iterators are not equal if the coiterators
                 * are not equal. This check proceeds by checking if the coiterators are
                 * all equal.
                 *
                 * Questions:
                 * 1. Does each coiterator check it with respect to the function `F`, or
                 * with their modified function, where certain indices are set to `true`?
                 * 2. [Derferencing] How should we dereference?
                 * 3. [Advancing] When we advance for a merge lattice with (i, j, k)
                 * we want to advance all coiterators starting at index i. Once we
                 * get to the next position, we want to advance all coiterators starting
                 * at index j, and so on. How do we do this?
                 * 
                 * For example: row 0 has no non-zero values, so row i==1 is the first
                 * advanced row. Then, we would need to advance column j to the first
                 * non-zero value, and then advance column k to the first non-zero value.
                 * However, column j and k need to be aware if a new row has occured?
                 * 
                 * 4. How can we initialize the coiterators during compile-time?
                 */
                {
                    // return !F<>::value;
                };

            inline bool operator==(iterator const& other) const noexcept
            {
                return !(*this != other);
            };
        };

        inline iterator begin() const noexcept
        /**
         * @brief Beginning of each index's coiterator.
         *
         */
        {
            return iterator{ *this,
                             std::apply([&](auto&... args) { return std::tuple(args.begin()...); },
                                        this->m_coiters) };
        }

        inline iterator end() const noexcept
        /**
         * @brief End of each index's coiterator.
         *
         */
        {
            return iterator{ *this,
                             std::apply([&](auto&... args) { return std::tuple(args.end()...); },
                                        this->m_coiters) };
        }
    };
}

#endif
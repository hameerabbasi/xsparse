#ifndef XSPARSE_TENSOR_HPP
#define XSPARSE_TENSOR_HPP

#include <tuple>
#include <vector>
#include <map>

#include <xsparse/util/container_traits.hpp>


namespace xsparse
{
    /**
     * @brief A (sparse) tensor is a collection of levels.
     *
     * @tparam DataType - the type of the data that is held (e.g. int) is a tensor of ints
     * @tparam Levels - a tuple of levels, which comprise this tensor.
     * @tparam Data - the data that is pointed to by the last level.
     *
     */
    template <typename DataType, class Levels, class Data>
    class Tensor;

    template <typename DataType, class... Levels, class Data>
    class Tensor<DataType, std::tuple<Levels...>, Data>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;
        Data& m_data;

        /*Private methods for public API.*/
        template <std::size_t I>
        inline constexpr auto get_size_level() const noexcept
        {
            return std::get<I>(m_levelsTuple).size();
        }

        template <std::size_t... I>
        inline constexpr auto get_shape_complete(
            [[maybe_unused]] std::index_sequence<I...> i) const noexcept
        /**
         * @brief Helper function to obtain size from each level.
         *
         * @details Performs a fold expression over `get_size_level` for every index in
         * the index sequence `i`.
         */
        {
            return std::make_tuple(get_size_level<I>()...);
        }

    public:
        using dtype = DataType;

        explicit inline Tensor(Levels&... levels, Data& data)
            : m_levelsTuple(std::tie(levels...))
            , m_data(data)
        {
            // XXX: we should add compile-time checks to make sure the tensor is appropriately
            // defined.
            // check if Data type is the same as DataType
            static_assert(std::is_same_v<DataType, typename Data::value_type>, "Data type and DataType must be the same.");
        }

        inline constexpr auto ndim() const noexcept
        /**
         * @brief Returns the dimensionality of the tensor (also called mode).
         *
         * @return constexpr std::size_t - the dimensionality of the tensor, which
         * is the size of the tuple of levels.
         */
        {
            return sizeof...(Levels);
        }

        inline auto shape() const noexcept
        {
            // e.g. for a tensor of shape (i, j, k) this should return (i, j, k)
            // get the size of each tensor mode and return as a tuple
            return get_shape_complete(
                std::make_index_sequence<std::tuple_size_v<decltype(m_levelsTuple)>>{});
        }

        inline auto get_levels() const noexcept
        {
            return m_levelsTuple;
        }

        inline auto get_levels() noexcept
        {
            return m_levelsTuple;
        }

        // TODO: support just iterating through index
        // e.g. if storage is (i, j, k) corresponding to (hashed, dense, compressed)
        // we would iterate hashed, then dense, then compressed?
        // we would iterate in a nested for loop (mainly think about what would be
        // in operator++)

    public:
        class iterator
            {
            private:
                coiteration_helper const& m_coiterHelper;
                std::tuple<typename Levels::iteration_helper::iterator...> iterators;
                IK min_ik;

                auto m_iterators_init() noexcept
                /**
                 * @brief Initialize iterators for each level in the tensor.
                 */
                {
                    return std::apply([&](auto&... args)
                                      { return std::tuple(args.begin()...); },
                                      this->m_levelsTuple);
                }

            public:
                using iterator_category = std::forward_iterator_tag;
                using reference = uintptr_t;

                explicit inline iterator() noexcept
                : iterators(m_iterators_init)
                {
                }

                template <class iter>
                inline void advance_iter(iter& i) const noexcept
                {
                    // advance iterator if it is ordered
                    if constexpr (iter::parent_type::LevelProperties::is_ordered)
                    {
                        if (static_cast<IK>(std::get<0>(*i)) == min_ik)
                        {
                            ++i;
                        }
                    }
                }

                inline reference operator*() const noexcept
                /**
                 * @brief Return the tuple of index and PK for each level.
                 * 
                 */
                {
                    // return { min_ik, PK_tuple };
                }

                inline iterator& operator++() noexcept
                /**
                 * @brief Increment the index through the tensor.
                 * 
                 * @details For example, for a 3D tensor with indices (i, j, k), 
                 * representing levels (A, B, C), then the iterator will increment
                 * through the tensor in the following order:
                 * 
                 * (0, 0, 0) -> (0, 0, 1) -> ... -> (0, 0, k) -> (0, 1, 0) 
                 * -> ... -> (0, i, k) -> (1, 0, 0) -> ... -> (i, k, k)
                 * 
                 */
                {
                    iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                inline bool operator!=(iterator const& other) const noexcept
                /**
                 * @brief Compare the iterators of all the levels associated with another tensor.
                 * 
                 */
                {
                };

                inline bool operator==(iterator const& other) const noexcept
                {
                    return !(*this != other);
                };
            };

            inline iterator begin() const noexcept
            {
                return iterator{ *this,
                                 std::apply([&](auto&... args)
                                            { return std::tuple(args.begin()...); },
                                            this->m_iterHelpers) };
            }

            inline iterator end() const noexcept
            {
                return iterator{ *this,
                                 std::apply([&](auto&... args)
                                            { return std::tuple(args.end()...); },
                                            this->m_iterHelpers) };
            }
        };
    };
}

#endif
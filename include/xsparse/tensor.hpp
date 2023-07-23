#ifndef XSPARSE_TENSOR_HPP
#define XSPARSE_TENSOR_HPP

#include <tuple>


namespace xsparse
{
    /**
     * @brief A (sparse) tensor is a collection of levels.
     *
     * @tparam DataType - the type of the data that is held (e.g. int) is a tensor of ints
     * @tparam Levels - a tuple of levels, which comprise this tensor.
     */
    template <class DataType, class... Levels>
    class Tensor;

    template <class DataType, class... Levels>
    class Tensor<DataType, std::tuple<Levels...>>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;

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
        explicit inline Tensor(Levels&... levels)
            : m_levelsTuple(std::tie(levels...))
        {
            // XXX: we should add compile-time checks to make sure the tensor is appropriately
            // defined.
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

        inline constexpr auto dtype() const noexcept
        {
            return static_cast<DataType*>(nullptr);
        }

        inline constexpr auto shape() const noexcept
        {
            // e.g. for a tensor of shape (i, j, k) this should return (i, j, k)
            // get the size of each tensor mode and return as a tuple
            return get_shape_complete(
                        std::make_index_sequence<std::tuple_size_v<decltype(m_levelsTuple)>>{});
        }

        inline constexpr auto get_levels() const noexcept
        {
            return m_levelsTuple;
        }

        inline auto get_levels() noexcept
        {
            return m_levelsTuple;
        }
    };
}

#endif
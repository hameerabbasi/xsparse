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
    template <class Levels, class Data>
    class Tensor;

    template <class... Levels, class Data>
    class Tensor<std::tuple<Levels...>, Data>
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
        using dtype = typename Data::value_type;

        explicit inline Tensor(Levels&... levels, Data& data)
            : m_levelsTuple(std::tie(levels...))
            , m_data(data)
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
    };
}

#endif

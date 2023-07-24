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
     * @tparam ContainerTraits - the container traits to use for the tensor.
     */
    template <typename DataType,
              class Levels,
              class ContainerTraits
              = util::container_traits<std::vector, std::unordered_set, std::unordered_map>>
    class Tensor;

    // TODO: datatype, and also ContainerTraits similar to the levels, where Elem is `DateType`.
    // Need a TVec of Data
    // - how do we append data to the tensor? -> container_traits helps us define this
    // - how do we read/write data to the tensor at a specific location?
    // - and how do we mush many of these together for the sake of multithreading?
    template <typename DataType, class... Levels, class ContainerTraits>
    class Tensor<DataType, std::tuple<Levels...>, ContainerTraits>
    {
        // using PosContainer = typename ContainerTraits::template Vec<DataType>;
        // using CrdContainer = typename ContainerTraits::template Vec<DataType>;

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
        using dtype = DataType;

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

        /**
         * @brief Get reference to data element at specific index tuple.
         *
         * @param index
         * @return DataType&
         */
        // inline DataType& get_data(std::tuple<std::size_t...> index) const noexcept
        // {
        //     return std::apply(
        //         [&](auto&&... levels) {
        //             return std::make_tuple(levels.get_data(index)...);
        //         },
        //         m_levelsTuple);
        // }

        // inline DataType& get_data(std::tuple<std::size_t...> index) noexcept
        // {
        //     return std::apply(
        //         [&](auto&&... levels) {
        //             return std::make_tuple(levels.get_data(index)...);
        //         },
        //         m_levelsTuple);
        // }

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
    };
}

#endif
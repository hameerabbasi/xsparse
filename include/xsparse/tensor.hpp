#ifndef XSPARSE_TENSOR_HPP
#define XSPARSE_TENSOR_HPP

#include <tuple>

namespace xsparse
{
    /**
     * @brief A (sparse) tensor is a collection of levels.
     *
     * @tparam Levels - a tuple of levels, which comprise this tensor.
     */
    template <class ContainerType, class... Levels>
    class Tensor;

    template <class ContainerType, class... Levels>
    class Tensor<ContainerType, std::tuple<Levels...>>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;

    public:
        // TODO; need to add template parameter ContainerType which is the type of the data
        // that is held (e.g. int) is a tensor of ints 
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

        inline constexpr auto shape() const noexcept
        {
            // TODO: this should return a tuple of the shapes of the tensor
        }

        inline const auto get_levels() const noexcept
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
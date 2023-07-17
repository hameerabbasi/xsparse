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
    template <class Levels>
    class Tensor;

    template <class... Levels>
    class Tensor<std::tuple<Levels...>>
    {
    private:
        std::tuple<Levels&...> const m_levelsTuple;

    public:
        explicit inline Tensor(Levels&... levels)
        : m_levelsTuple(std::tie(levels...))
        {
            // XXX: we should add compile-time checks to make sure the tensor is appropriately defined.
        }

        inline constexpr auto get_dimension() const noexcept
        /**
         * @brief Returns the dimensionality of the tensor.
         * 
         * @return constexpr std::size_t - the dimensionality of the tensor, which
         * is the size of the tuple of levels.
         */
        {
            return sizeof...(Levels);
        }

        inline constexpr auto get_levels() const noexcept
        {
            return m_levelsTuple;
        }
    };
}

#endif
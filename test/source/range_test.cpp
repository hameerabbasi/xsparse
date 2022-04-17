#include <doctest/doctest.h>

#include <tuple>
#include <vector>

#include <xsparse/levels/range.hpp>
#include <xsparse/version.h>

TEST_CASE("Compressed-BaseCase")
{
    // constexpr uintptr_t SIZE = 4;
    constexpr uintptr_t SIZE_N = 4;
    constexpr uintptr_t SIZE_M = 6;
    constexpr uint8_t ZERO = 0;

    std::vector<int16_t> const offset{ -3, -1, 0, 1 };

    xsparse::levels::range<std::tuple<>, uintptr_t, uintptr_t, std::vector<int16_t>> r{ SIZE_N,
                                                                                        SIZE_M,
                                                                                        offset };

    // uintptr_t loop = 0;

    for (auto const [i, p] : r.iter_helper(std::make_tuple(), ZERO))
    {
        //
    }
}
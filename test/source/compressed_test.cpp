#include <doctest/doctest.h>

#include <tuple>
#include <vector>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/version.h>

TEST_CASE("Compressed-BaseCase")
{
    constexpr uintptr_t SIZE = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> pos{ 0, 5 };
    std::vector<uintptr_t> crd{ 20, 30, 50, 60, 70 };

    xsparse::levels::compressed<std::tuple<>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s(SIZE, pos, crd);

    uintptr_t i = 0;
    for (auto const [crd_i, pos_i] : s.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos_i);
        CHECK(crd[i] == crd_i);
        ++i;
    }
    CHECK(i == pos.back());
}
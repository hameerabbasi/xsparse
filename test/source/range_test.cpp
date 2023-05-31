#include <doctest/doctest.h>

#include <tuple>
#include <vector>

#include <xsparse/levels/range.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/offset.hpp>
#include <xsparse/version.h>

#include <iostream>


TEST_CASE("Range-DIA")
{
    constexpr uintptr_t SIZE = 4;
    constexpr uintptr_t SIZE_N = 4;
    constexpr uintptr_t SIZE_M = 6;
    constexpr uint8_t ZERO = 0;

    std::vector<int16_t> const offset{ -3, -1, 0, 1 };

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d{ SIZE };

    xsparse::levels::range<std::tuple<decltype(d)>, uintptr_t, int16_t> r{ SIZE_N, SIZE_M, offset };
    xsparse::levels::offset<std::tuple<decltype(r), decltype(d)>, uintptr_t, int16_t> o{ SIZE,
                                                                                         offset };

    uintptr_t l1 = 0;

    for (auto const [i1, p1] : d.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(l1 == i1);
        uintptr_t l2 = std::max(0, -offset[l1]);
        for (auto const [i2, p2] : r.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(l2 + SIZE_N * l1 == p2);
            CHECK(l2 == i2);
            for (auto const [i3, p3] : o.iter_helper(std::make_tuple(i2, i1), p2))
            {
                CHECK(p2 == p3);
                CHECK(l2 + offset[l1] == i3);
            }
            ++l2;
        }
        CHECK(l2 == std::min(SIZE_N, SIZE_M - offset[l1]));
        ++l1;
    }
    CHECK(l1 == SIZE);

    // Check basic properties of all range levels
    CHECK(!r.LevelProperty().is_full);
    CHECK(r.LevelProperty().is_ordered);
    CHECK(r.LevelProperty().is_unique);
    CHECK(!r.LevelProperty().is_branchless);
    CHECK(!r.LevelProperty().is_compact);

    // Check basic properties of all offset levels
    CHECK(!o.LevelProperty().is_full);
    CHECK(o.LevelProperty().is_ordered);
    CHECK(o.LevelProperty().is_unique);
    CHECK(o.LevelProperty().is_branchless);
    CHECK(!o.LevelProperty().is_compact);
}

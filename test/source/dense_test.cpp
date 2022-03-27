#include <doctest/doctest.h>

#include <tuple>

#include <xsparse/levels/dense.hpp>
#include <xsparse/version.h>

TEST_CASE("Dense-BaseCase")
{
    constexpr uintptr_t SIZE = 5;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d{ SIZE };

    uintptr_t loop = 0;
    for (auto const [i, p] : d.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(loop == p);
        CHECK(loop == i);
        ++loop;
    }
    CHECK(loop == SIZE);
}

TEST_CASE("Dense-2D")
{
    constexpr uintptr_t SIZE1 = 5;
    constexpr uintptr_t SIZE2 = 4;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1{ SIZE1 };
    xsparse::levels::dense<std::tuple<decltype(d1)>, uintptr_t, uintptr_t> d2{ SIZE2 };


    uintptr_t l1 = 0;
    for (auto const [i1, p1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(l1 == i1);
        uintptr_t l2 = 0;
        for (auto const [i2, p2] : d2.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(l1 * SIZE2 + l2 == p2);
            CHECK(l2 == i2);
            ++l2;
        }
        CHECK(l2 == SIZE2);
        ++l1;
    }
    CHECK(l1 == SIZE1);
}

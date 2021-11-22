#include <doctest/doctest.h>

#include <tuple>

#include <xsparse/levels/dense.hpp>
#include <xsparse/version.h>

TEST_CASE("Dense-BaseCase")
{
    constexpr uintptr_t SIZE = 5;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d(SIZE);

    uintptr_t i = 0;
    for (auto const [crd, pos] : d.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos);
        CHECK(i == crd);
        ++i;
    }
    CHECK(i == SIZE);
}

TEST_CASE("Dense-2D")
{
    constexpr uintptr_t SIZE1 = 5;
    constexpr uintptr_t SIZE2 = 4;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1(SIZE1);
    xsparse::levels::dense<std::tuple<decltype(d1)>, uintptr_t, uintptr_t> d2(SIZE2);


    uintptr_t i = 0;
    for (auto const [crd1, pos1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos1);
        CHECK(i == crd1);
        uintptr_t j = 0;
        for (auto const [crd2, pos2] : d2.iter_helper(std::make_tuple(crd1), pos1))
        {
            CHECK(i * SIZE2 + j == pos2);
            CHECK(j == crd2);
            ++j;
        }
        CHECK(j == SIZE2);
        ++i;
    }
    CHECK(i == SIZE1);
}

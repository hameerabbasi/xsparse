#include <doctest/doctest.h>

#include <tuple>

#include <xsparse/xsparse.hpp>
#include <xsparse/version.h>

TEST_CASE("Dense-BaseCase")
{
    constexpr uintptr_t SIZE = 5;
    constexpr uintptr_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d(SIZE);

    uintptr_t i = 0;
    for (auto const [val, pos] : d.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos);
        CHECK(i == val);
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
    for (auto const [val1, pos1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos1);
        CHECK(i == val1);
        uintptr_t j = 0;
        for (auto const [val2, pos2] : d2.iter_helper(std::make_tuple(val1), pos1))
        {
            CHECK(i * SIZE2 + j == pos2);
            CHECK(j == val2);
            ++j;
        }
        CHECK(j == SIZE2);
        ++i;
    }
    CHECK(i == SIZE1);
}

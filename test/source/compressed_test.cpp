#include <doctest/doctest.h>

#include <tuple>
#include <vector>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/version.h>

TEST_CASE("Compressed-BaseCase")
{
    constexpr uintptr_t SIZE = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 5 };
    std::vector<uintptr_t> const crd{ 20, 30, 50, 60, 70 };

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

TEST_CASE("Compressed-CSR")
{
    constexpr uintptr_t SIZE1 = 3;
    constexpr uintptr_t SIZE2 = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 2, 5, 9 };
    std::vector<uintptr_t> const crd{ 20, 50, 30, 40, 70, 10, 60, 80, 90 };

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1(SIZE1);
    xsparse::levels::compressed<std::tuple<decltype(d1)>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s2(SIZE2, pos, crd);


    uintptr_t i = 0;
    for (auto const [crd1, pos1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(i == pos1);
        CHECK(i == crd1);
        uintptr_t j = 0;
        for (auto const [crd2, pos2] : s2.iter_helper(std::make_tuple(crd1), pos1))
        {
            CHECK(pos[i] + j == pos2);
            CHECK(crd[pos2] == crd2);
            ++j;
        }
        CHECK(j == pos[i + 1] - pos[i]);
        ++i;
    }
    CHECK(i == SIZE1);
}

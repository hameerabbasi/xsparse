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
        s{ SIZE, pos, crd };

    uintptr_t l = 0;
    for (auto const [i, p] : s.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l == p);
        CHECK(crd[l] == i);
        ++l;
    }
    CHECK(l == pos.back());
}

TEST_CASE("Compressed-CSR")
{
    constexpr uintptr_t SIZE1 = 3;
    constexpr uintptr_t SIZE2 = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 2, 5, 9 };
    std::vector<uintptr_t> const crd{ 20, 50, 30, 40, 70, 10, 60, 80, 90 };

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1{ SIZE1 };
    xsparse::levels::compressed<std::tuple<decltype(d1)>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s2{ SIZE2, pos, crd };


    uintptr_t l1 = 0;
    for (auto const [i1, p1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(l1 == i1);
        uintptr_t l2 = 0;
        for (auto const [i2, p2] : s2.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(pos[l1] + l2 == p2);
            CHECK(crd[p2] == i2);
            ++l2;
        }
        CHECK(l2 == pos[l1 + 1] - pos[l1]);
        ++l1;
    }
    CHECK(l1 == SIZE1);
}

TEST_CASE("Compressed-DCSR")
{
    constexpr uintptr_t SIZE1 = 100;
    constexpr uintptr_t SIZE2 = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos1{ 0, 3 };
    std::vector<uintptr_t> const crd1{ 20, 50, 70 };
    std::vector<uintptr_t> const pos2{ 0, 2, 5, 9 };
    std::vector<uintptr_t> const crd2{ 20, 50, 30, 40, 70, 10, 60, 80, 90 };

    xsparse::levels::compressed<std::tuple<>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s1{ SIZE1, pos1, crd1 };
    xsparse::levels::compressed<std::tuple<decltype(s1)>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s2{ SIZE2, pos2, crd2 };


    uintptr_t l1 = 0;
    for (auto const [i1, p1] : s1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(crd1[l1] == i1);
        uintptr_t l2 = 0;
        for (auto const [i2, p2] : s2.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(pos2[l1] + l2 == p2);
            CHECK(crd2[p2] == i2);
            ++l2;
        }
        CHECK(l2 == pos2[l1 + 1] - pos2[l1]);
        ++l1;
    }
    CHECK(l1 == pos1.back());
}

TEST_CASE("Compressed-Append")
{
    constexpr uintptr_t SIZE = 0;
    constexpr uint8_t ZERO = 0;
    constexpr uintptr_t SIZE_PREV = 1;

    std::vector<uintptr_t> pos{ 0 };
    std::vector<uintptr_t> crd;

    xsparse::levels::compressed<std::tuple<>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s{ SIZE, pos, crd };

    s.append_init(SIZE_PREV);
    s.append_edges(0, 0, 4);
    s.append_coord(0);
    s.append_coord(1);
    s.append_coord(4);
    s.append_coord(6);
    s.append_finalize(SIZE_PREV);

    uintptr_t l = 0;
    for (auto const [i, p] : s.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l == p);
        CHECK(crd[l] == i);
        ++l;
    }
    CHECK(l == pos.back());
}

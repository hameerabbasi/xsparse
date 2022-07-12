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

TEST_CASE("Compressed-CSR-Append")
{
    constexpr uintptr_t SIZE1 = 4;
    constexpr uintptr_t SIZE2 = 100;
    constexpr uint8_t ZERO = 0;
    constexpr uint8_t ONE = 1;

    std::vector<uintptr_t> const pos_holder{ 0, 2, 4, 4, 7 };
    std::vector<uintptr_t> const crd_holder{ 0, 1, 0, 1, 0, 3, 4 };

    std::vector<uintptr_t> pos{ 0 };
    std::vector<uintptr_t> crd;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1{ SIZE1 };
    xsparse::levels::compressed<std::tuple<decltype(d1)>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        s2{ SIZE2, pos, crd };

    s2.append_init(SIZE1);

    uintptr_t pkm1 = 0;
    for (int i = 1; i < pos_holder.size(); ++i)
    {
        s2.append_edges(pkm1, ZERO, pos_holder[i] - pos_holder[i - 1]);
        ++pkm1;
    }

    for (auto const c : crd_holder)
    {
        s2.append_coord(c);
    }

    s2.append_finalize(SIZE1);

    uintptr_t l1 = 0;
    for (auto const [i1, p1] : d1.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(l1 == i1);
        uintptr_t l2 = 0;
        for (auto const [i2, p2] : s2.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(pos_holder[l1] + l2 == p2);
            CHECK(crd_holder[p2] == i2);
            ++l2;
        }
        CHECK(l2 == pos_holder[l1 + 1] - pos_holder[l1]);
        ++l1;
    }
    CHECK(l1 == d1.size(ONE));
}

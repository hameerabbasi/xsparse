#include <doctest/doctest.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <map>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/singleton.hpp>

#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>


TEST_CASE("Singleton-BaseCase")
{
    constexpr uintptr_t SIZE = 20;
    constexpr uint8_t ZERO = 0;
    std::vector<uintptr_t> const crd1{ 0, 1, 0, 1, 0, 3, 4 };

    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s{ SIZE, crd1 };

    uintptr_t l2 = 0;
    for (auto const [i2, p2] : s.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l2 == p2);
        CHECK(crd1[l2] == i2);
        ++l2;
    }

    // Check basic stric properties of all singleton levels
    CHECK(decltype(s)::LevelProperties::is_branchless);
    CHECK(decltype(s)::LevelProperties::is_compact);
}

TEST_CASE("Singleton-COO")
{
    constexpr uintptr_t SIZE = 100;
    constexpr uintptr_t SIZE1 = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 7 };
    std::vector<uintptr_t> const crd0{ 0, 0, 1, 1, 3, 3, 3 };
    std::vector<uintptr_t> const crd1{ 0, 1, 0, 1, 0, 3, 4 };

    xsparse::levels::compressed<std::tuple<>, uintptr_t, uintptr_t> c{ SIZE, pos, crd0 };

    xsparse::levels::singleton<std::tuple<decltype(c)>, uintptr_t, uintptr_t> s{ SIZE1, crd1 };

    uintptr_t l1 = 0;
    for (auto const [i1, p1] : c.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(crd0[l1] == i1);
        uintptr_t l2 = p1;
        for (auto const [i2, p2] : s.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(l2 == p2);
            CHECK(crd1[l2] == i2);
            ++l2;
        }
        CHECK(l2 == p1 + 1);
        ++l1;
    }
    CHECK(l1 == pos.back());
}

TEST_CASE("Singleton-COO-3D")
{
    constexpr uintptr_t SIZE = 0;
    constexpr uintptr_t SIZE1 = 100;
    constexpr uintptr_t SIZE2 = 1;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 8 };
    std::vector<uintptr_t> const crd0{ 0, 0, 0, 2, 2, 2, 2, 2 };
    std::vector<uintptr_t> const crd1{ 0, 0, 2, 0, 2, 2, 3, 3 };
    std::vector<uintptr_t> const crd2{ 0, 1, 1, 1, 0, 1, 0, 1 };

    xsparse::levels::compressed<
        std::tuple<>,
        uintptr_t,
        uintptr_t,
        xsparse::util::container_traits<std::vector, std::set, std::unordered_map>>
        c{ SIZE, pos, crd0 };

    xsparse::levels::singleton<std::tuple<decltype(c)>,
                               uintptr_t,
                               uintptr_t,
                               xsparse::util::container_traits<std::vector, std::set, std::map>>
        s{ SIZE1, crd1 };

    xsparse::levels::singleton<std::tuple<decltype(s)>,
                               uintptr_t,
                               uintptr_t,
                               xsparse::util::container_traits<std::vector, std::set, std::map>,
                               xsparse::level_properties<false, false, false, true, true>>
        s1{ SIZE2, crd2 };

    uintptr_t l1 = 0;
    for (auto const [i1, p1] : c.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(crd0[l1] == i1);
        uintptr_t l2 = p1;
        for (auto const [i2, p2] : s.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(l2 == p2);
            CHECK(crd1[l2] == i2);
            uintptr_t l3 = p2;
            for (auto const [i3, p3] : s1.iter_helper(std::make_tuple(i2), p2))
            {
                CHECK(l3 == p3);
                CHECK(crd2[l3] == i3);
                ++l3;
            }
            CHECK(l3 == p2 + 1);
            ++l2;
        }
        CHECK(l2 == p1 + 1);
        ++l1;
    }
    CHECK(l1 == pos.back());
}

TEST_CASE("Singleton-COO-Append")
{
    constexpr uintptr_t SIZE = 1;
    constexpr uintptr_t SIZE1 = 1;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos_holder{ 0, 7 };
    std::vector<uintptr_t> const crd0_holder{ 0, 0, 1, 1, 3, 3, 3 };
    std::vector<uintptr_t> const crd1_holder{ 0, 1, 0, 1, 0, 3, 4 };

    std::vector<uintptr_t> const pos{ 0 };
    std::vector<uintptr_t> const crd0;
    std::vector<uintptr_t> const crd1;

    xsparse::levels::compressed<std::tuple<>, uintptr_t, uintptr_t> c{ SIZE, pos, crd0 };

    xsparse::levels::singleton<std::tuple<decltype(c)>, uintptr_t, uintptr_t> s{ SIZE1, crd1 };

    c.append_init(SIZE);

    uintptr_t pkm1 = 0;
    for (size_t i = 0; i < pos_holder.size() - 1; ++i)
    {
        c.append_edges(pkm1, ZERO, pos_holder[i + 1] - pos_holder[i]);
        ++pkm1;
    }

    for (auto const coord : crd0_holder)
    {
        c.append_coord(coord);
    }

    c.append_finalize(SIZE);

    for (auto const coord : crd1_holder)
    {
        s.append_coord(coord);
    }

    uintptr_t l1 = 0;
    for (auto const [i1, p1] : c.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(crd0_holder[l1] == i1);
        uintptr_t l2 = p1;
        for (auto const [i2, p2] : s.iter_helper(std::make_tuple(i1), p1))
        {
            CHECK(l2 == p2);
            CHECK(crd1_holder[l2] == i2);
            ++l2;
        }
        CHECK(l2 == p1 + 1);
        ++l1;
    }
    CHECK(l1 == pos_holder.back());
}

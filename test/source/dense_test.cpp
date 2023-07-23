#include <doctest/doctest.h>

#include <tuple>

#include <xsparse/levels/dense.hpp>
#include <xsparse/version.h>
#include <xsparse/level_properties.hpp>
#include <xsparse/level_capabilities/locate.hpp>

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

    // Check basic strict properties of all dense levels
    static_assert(decltype(d)::LevelProperties::is_full);
    static_assert(!decltype(d)::LevelProperties::is_branchless);
    static_assert(decltype(d)::LevelProperties::is_compact);

    // the dense level does not have locate
    static_assert(!has_locate_v<decltype(d)>);
}

TEST_CASE("Dense-2D")
{
    constexpr uintptr_t SIZE1 = 5;
    constexpr uintptr_t SIZE2 = 4;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>,
                           uintptr_t,
                           uintptr_t,
                           xsparse::level_properties<true, true, false, false, true>>
        d1{ SIZE1 };
    xsparse::levels::dense<std::tuple<decltype(d1)>, uintptr_t, uintptr_t> d2{ SIZE2 };

    CHECK(d1.size() == SIZE1);
    CHECK(d2.size() == SIZE2);
    
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

TEST_CASE("Dense-2D-Size")
{
    constexpr uintptr_t SIZE1 = 6;
    constexpr uintptr_t SIZE2 = 6;
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1{ SIZE1 };
    xsparse::levels::dense<std::tuple<decltype(d1)>,
                           uintptr_t,
                           uintptr_t,
                           xsparse::level_properties<true, true, true, false, true>>
        d2{ SIZE2 };

    CHECK(d1.size() == SIZE1);
    CHECK(d2.size() == SIZE2);

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

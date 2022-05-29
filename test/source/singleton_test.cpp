#include <doctest/doctest.h>

#include <iostream>
#include <vector>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/singleton.hpp>

TEST_CASE("Singleton-COO")
{
    constexpr uintptr_t SIZE = 100;
    constexpr uintptr_t SIZE1 = 100;
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const pos{ 0, 7 };
    std::vector<uintptr_t> const crd{ 0, 0, 1, 1, 3, 3, 3 };
    std::vector<uintptr_t> const crd1{ 0, 1, 0, 1, 0, 3, 4 };

    xsparse::levels::compressed<std::tuple<>,
                                uintptr_t,
                                uintptr_t,
                                std::vector<uintptr_t>,
                                std::vector<uintptr_t>>
        c{ SIZE, pos, crd };

    xsparse::levels::
        singleton<std::tuple<decltype(c)>, uintptr_t, uintptr_t, std::vector<uintptr_t>>
            s{ SIZE1, crd1 };

    uintptr_t l1 = 0;
    for (auto const [i1, p1] : c.iter_helper(std::make_tuple(), ZERO))
    {
        CHECK(l1 == p1);
        CHECK(crd[l1] == i1);
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

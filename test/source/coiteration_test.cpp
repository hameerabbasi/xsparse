#include <doctest/doctest.h>

#include <tuple>
#include <vector>
#include <functional>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/singleton.hpp>
#include <xsparse/version.h>

#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>
#include <xsparse/level_capabilities/co_iteration.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>

TEST_CASE("Coiteration-Dense-Dense")
{
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s1{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s2{ 5 };

    auto fn = [](std::tuple<bool, bool> t) constexpr
    {
        return std::get<0>(t) && std::get<1>(t);
    };

    xsparse::level_capabilities::Coiterate<std::function<bool(std::tuple<bool, bool>)>,
                                           uintptr_t,
                                           uintptr_t,
                                           std::tuple<decltype(s1), decltype(s2)>,
                                           std::tuple<>>
        coiter;

    auto it1 = s1.iter_helper(std::make_tuple(), ZERO).begin();
    auto it2 = s2.iter_helper(std::make_tuple(), ZERO).begin();

    auto end1 = s1.iter_helper(std::make_tuple(), ZERO).end();
    auto end2 = s2.iter_helper(std::make_tuple(), ZERO).end();


    for (auto const [ik, pk_tuple] : coiter.coiter_helper(fn, std::make_tuple(), ZERO, s1, s2))
    {
        auto [i1, p1] = *it1;
        auto [i2, p2] = *it2;
        uintptr_t l = std::min(i1, i2);
        CHECK(ik == l);
        if (i1 == l)
        {
            CHECK(p1 == std::get<0>(pk_tuple).value());
            ++it1;
        }
        if (i2 == l)
        {
            CHECK(p2 == std::get<1>(pk_tuple).value());
            ++it2;
        }
    }
    CHECK(it1 == end1);
    CHECK(it2 == end2);
}

TEST_CASE("Coiteration-Dense-Dense-Dense")
{
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s1{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s2{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s3{ 4 };

    auto fn = [](std::tuple<bool, bool, bool> t) constexpr
    {
        return (std::get<0>(t) && std::get<1>(t)) || (std::get<2>(t));
    };

    xsparse::level_capabilities::Coiterate<std::function<bool(std::tuple<bool, bool, bool>)>,
                                           uintptr_t,
                                           uintptr_t,
                                           std::tuple<decltype(s1), decltype(s2), decltype(s3)>,
                                           std::tuple<>>
        coiter;

    auto it1 = s1.iter_helper(std::make_tuple(), ZERO).begin();
    auto it2 = s2.iter_helper(std::make_tuple(), ZERO).begin();
    auto it3 = s3.iter_helper(std::make_tuple(), ZERO).begin();

    auto end1 = s1.iter_helper(std::make_tuple(), ZERO).end();
    auto end2 = s2.iter_helper(std::make_tuple(), ZERO).end();
    auto end3 = s3.iter_helper(std::make_tuple(), ZERO).end();

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(fn, std::make_tuple(), ZERO, s1, s2, s3))
    {
        auto [i1, p1] = *it1;
        auto [i2, p2] = *it2;
        auto [i3, p3] = *it3;

        uintptr_t l = std::min(i1, std::min(i2, i3));
        CHECK(ik == l);
        if (i1 == l)
        {
            CHECK(p1 == std::get<0>(pk_tuple).value());
            ++it1;
        }
        if (i2 == l)
        {
            CHECK(p2 == std::get<1>(pk_tuple).value());
            ++it2;
        }
        if (i3 == l)
        {
            CHECK(p3 == std::get<2>(pk_tuple).value());
            ++it3;
        }
    }
    CHECK(fn(std::tuple(it1 == end1, it2 == end2, it3 == end3)) == true);
}

TEST_CASE("Coiteration-Singleton-Singleton-Dense-Dense")
{
    constexpr uint8_t ZERO = 0;

    std::vector<uintptr_t> const crd1{ 1, 2, 3, 4 };
    std::vector<uintptr_t> const crd2{ 4, 5, 8 };

    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s1{ 4, crd1 };
    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s2{ 3, crd2 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s3{ 4 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s4{ 5 };

    auto fn = [](std::tuple<bool, bool, bool, bool> t) constexpr
    {
        return (std::get<0>(t) || std::get<2>(t)) || (std::get<1>(t) || std::get<3>(t));
    };

    xsparse::level_capabilities::Coiterate<
        std::function<bool(std::tuple<bool, bool, bool, bool>)>,
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2), decltype(s3), decltype(s4)>,
        std::tuple<>>
        coiter;

    auto it1 = s1.iter_helper(std::make_tuple(), ZERO).begin();
    auto it2 = s2.iter_helper(std::make_tuple(), ZERO).begin();
    auto it3 = s3.iter_helper(std::make_tuple(), ZERO).begin();
    auto it4 = s4.iter_helper(std::make_tuple(), ZERO).begin();

    auto end1 = s1.iter_helper(std::make_tuple(), ZERO).end();
    auto end2 = s2.iter_helper(std::make_tuple(), ZERO).end();
    auto end3 = s3.iter_helper(std::make_tuple(), ZERO).end();
    auto end4 = s4.iter_helper(std::make_tuple(), ZERO).end();

    for (auto const [ik, pk_tuple] :
         coiter.coiter_helper(fn, std::make_tuple(), ZERO, s1, s2, s3, s4))
    {
        auto [i1, p1] = *it1;
        auto [i2, p2] = *it2;
        auto [i3, p3] = *it3;
        auto [i4, p4] = *it4;

        uintptr_t l = std::min(std::min(i1, i2), std::min(i3, i4));
        CHECK(ik == l);
        if (i1 == l)
        {
            CHECK(p1 == std::get<0>(pk_tuple).value());
            ++it1;
        }
        if (i2 == l)
        {
            CHECK(p2 == std::get<1>(pk_tuple).value());
            ++it2;
        }
        if (i3 == l)
        {
            CHECK(p3 == std::get<2>(pk_tuple).value());
            ++it3;
        }
        if (i4 == l)
        {
            CHECK(p4 == std::get<3>(pk_tuple).value());
            ++it4;
        }
    }
    CHECK(fn(std::tuple(it1 == end1, it2 == end2, it3 == end3, it4 == end4)) == true);
}

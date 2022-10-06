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
        uintptr_t l = std::min(std::get<0>(*it1), std::get<0>(*it2));
        CHECK(ik == l);
        if (std::get<0>(*it1) == l)
        {
            CHECK(std::get<1>(*it1) == std::get<0>(pk_tuple));
            ++it1;
        }
        if (std::get<0>(*it2) == l)
        {
            CHECK(std::get<1>(*it2) == std::get<1>(pk_tuple));
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
        uintptr_t l = std::min(std::get<0>(*it1), std::min(std::get<0>(*it2), std::get<0>(*it3)));
        CHECK(ik == l);
        if (std::get<0>(*it1) == l)
        {
            CHECK(std::get<1>(*it1) == std::get<0>(pk_tuple));
            ++it1;
        }
        if (std::get<0>(*it2) == l)
        {
            CHECK(std::get<1>(*it2) == std::get<1>(pk_tuple));
            ++it2;
        }
        if (std::get<0>(*it3) == l)
        {
            CHECK(std::get<1>(*it3) == std::get<2>(pk_tuple));
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
        uintptr_t l = std::min(std::min(std::get<0>(*it1), std::get<0>(*it2)),
                               std::min(std::get<0>(*it3), std::get<0>(*it4)));
        CHECK(ik == l);
        if (std::get<0>(*it1) == l)
        {
            CHECK(std::get<1>(*it1) == std::get<0>(pk_tuple));
            ++it1;
        }
        if (std::get<0>(*it2) == l)
        {
            CHECK(std::get<1>(*it2) == std::get<1>(pk_tuple));
            ++it2;
        }
        if (std::get<0>(*it3) == l)
        {
            CHECK(std::get<1>(*it3) == std::get<2>(pk_tuple));
            ++it3;
        }
        if (std::get<0>(*it4) == l)
        {
            CHECK(std::get<1>(*it4) == std::get<3>(pk_tuple));
            ++it4;
        }
    }
    CHECK(fn(std::tuple(it1 == end1, it2 == end2, it3 == end3, it4 == end4)) == true);
}

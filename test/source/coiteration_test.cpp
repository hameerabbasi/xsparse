#include <doctest/doctest.h>

#include <tuple>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/singleton.hpp>
#include <xsparse/levels/hashed.hpp>
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

    auto fn = [](std::tuple<bool, bool> t) constexpr { return std::get<0>(t) && std::get<1>(t); };

    xsparse::level_capabilities::Coiterate<std::function<bool(std::tuple<bool, bool>)>,
                                           uintptr_t,
                                           uintptr_t,
                                           std::tuple<decltype(s1), decltype(s2)>,
                                           std::tuple<>>
        coiter(fn, s1, s2);

    auto it_helper1 = s1.iter_helper(std::make_tuple(), ZERO);
    auto it1 = it_helper1.begin();
    auto it_helper2 = s2.iter_helper(std::make_tuple(), ZERO);
    auto it2 = it_helper2.begin();

    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), ZERO))
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
    CHECK(fn(std::tuple(it1 == end1, it2 == end2)) == true);
}

TEST_CASE("Coiteration-Dense-Dense-Dense")
{
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s1{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s2{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s3{ 5 };

    auto fn = [](std::tuple<bool, bool, bool> t) constexpr
    { return (std::get<0>(t) && std::get<1>(t)) || (std::get<2>(t)); };

    xsparse::level_capabilities::Coiterate<std::function<bool(std::tuple<bool, bool, bool>)>,
                                           uintptr_t,
                                           uintptr_t,
                                           std::tuple<decltype(s1), decltype(s2), decltype(s3)>,
                                           std::tuple<>>
        coiter(fn, s1, s2, s3);

    auto it_helper1 = s1.iter_helper(std::make_tuple(), ZERO);
    auto it1 = it_helper1.begin();
    auto it_helper2 = s2.iter_helper(std::make_tuple(), ZERO);
    auto it2 = it_helper2.begin();
    auto it_helper3 = s3.iter_helper(std::make_tuple(), ZERO);
    auto it3 = it_helper3.begin();

    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();
    auto end3 = it_helper3.end();

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), ZERO))
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

    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s1{ 5, crd1 };
    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s2{ 5, crd2 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s3{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s4{ 5 };

    auto fn = [](std::tuple<bool, bool, bool, bool> t) constexpr
    { return (std::get<0>(t) || std::get<2>(t)) || (std::get<1>(t) || std::get<3>(t)); };

    xsparse::level_capabilities::Coiterate<
        std::function<bool(std::tuple<bool, bool, bool, bool>)>,
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2), decltype(s3), decltype(s4)>,
        std::tuple<>>
        coiter(fn, s1, s2, s3, s4);

    auto it_helper1 = s1.iter_helper(std::make_tuple(), ZERO);
    auto it1 = it_helper1.begin();
    auto it_helper2 = s2.iter_helper(std::make_tuple(), ZERO);
    auto it2 = it_helper2.begin();
    auto it_helper3 = s3.iter_helper(std::make_tuple(), ZERO);
    auto it3 = it_helper3.begin();
    auto it_helper4 = s4.iter_helper(std::make_tuple(), ZERO);
    auto it4 = it_helper4.begin();

    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();
    auto end3 = it_helper3.end();
    auto end4 = it_helper4.end();

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), ZERO))
    {
        if (it1 != end1 && it2 != end2 && it3 != end3 && it4 != end4)
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
    }
    CHECK(fn(std::tuple(it1 == end1, it2 == end2, it3 == end3, it4 == end4)) == true);
}

TEST_CASE("Coiteration-Dense-Hashed-ConjunctiveMerge")
{
    /* Test coiteration for dense and hashed formats.

    A conjunctive merge requires coiterating over a dense and hashed format. This
    test checks that the coiteration is done correctly. The test proceeds as follows:

    - If one of the levels is unordered (e.g. hashed, or singleton), then the
      coiteration is done by iterating over the ordered level, and then looking up
      the corresponding value in the unordered level.
    - The coiteration stops when the end of the ordered (i.e. dense) level is reached.

    This test checks that the lookup is done correctly.
    */
    constexpr uint8_t ZERO = 0;

    std::unordered_map<uintptr_t, uintptr_t> const umap1{ { 0, 1 }, { 2, 0 }, { 1, 2 } };
    std::vector<std::unordered_map<uintptr_t, uintptr_t>> const crd0{ umap1 };

    // initialize the two levels to be coiterated
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> dense_level{ 5 };
    xsparse::levels::hashed<
        std::tuple<>,
        uintptr_t,
        uintptr_t,
        xsparse::util::container_traits<std::vector, std::unordered_set, std::unordered_map>,
        xsparse::level_properties<false, false, false, false, false>>
        hash_level{ 5, crd0 };

    // define a conjunctive function
    auto fn = [](std::tuple<bool, bool> t) constexpr { return (std::get<0>(t) && std::get<1>(t)); };

    xsparse::level_capabilities::Coiterate<std::function<bool(std::tuple<bool, bool>)>,
                                           uintptr_t,
                                           uintptr_t,
                                           std::tuple<decltype(dense_level), decltype(hash_level)>,
                                           std::tuple<>>
        coiter(fn, dense_level, hash_level);

    // define iteration helper through dense and hashed level
    auto it_helper1 = dense_level.iter_helper(std::make_tuple(), ZERO);
    auto it_helper2 = hash_level.iter_helper(ZERO);

    // initialize iterators for dense and hashed level
    auto it1 = it_helper1.begin();
    auto it2 = it_helper2.begin();
    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();

    // when co-iterating over levels that are unordered (i.e. hashed), then we use locate to check
    // if the index exists in the hashed level. If not, then we skip it.
    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), ZERO))
    {
        // get the index and pointer from the levels involved in co-iteration
        auto [i1, p1] = *it1;

        // should only iterate over the ordered dense level
        uintptr_t l = i1;
        CHECK(ik == l);

        // check that neither level has reached the end
        if (it1 != end1)
        {
            // if both levels have a non-zero value at index "i1", then those values
            // should be present in the co-iterated tuple
            if (i1 == l && hash_level.locate(p1, i1) != std::nullopt)
            {
                CHECK(p1 == std::get<0>(pk_tuple).value());
            }

            // increment through the dense level always
            ++it1;
        }
    }

    // check that the dense level should've reached its end
    CHECK((it1 == end1) == true);
}

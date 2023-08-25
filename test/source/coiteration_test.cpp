#include <doctest/doctest.h>

#include <tuple>
#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/singleton.hpp>
#include <xsparse/levels/hashed.hpp>
#include <xsparse/version.h>

#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>
#include <xsparse/level_capabilities/co_iteration.hpp>
#include <xsparse/level_capabilities/coordinate_iterate.hpp>
#include <xsparse/util/template_utils.hpp>


TEST_CASE("Coiteration-Dense-Dense")
{
    constexpr uint8_t ZERO = 0;

    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s1{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s2{ 5 };

    auto fn = [](std::tuple<bool, bool> t) constexpr { return std::get<0>(t) && std::get<1>(t); };

    xsparse::level_capabilities::Coiterate<
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2)>,
        std::tuple<>,
        std::tuple<>>
        coiter(fn, s1, s2);

    auto it_helper1 = s1.iter_helper(std::make_tuple(), ZERO);
    auto it1 = it_helper1.begin();
    auto it_helper2 = s2.iter_helper(std::make_tuple(), ZERO);
    auto it2 = it_helper2.begin();

    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), std::make_tuple(ZERO)))
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

    xsparse::level_capabilities::Coiterate<
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2), decltype(s3)>,
        std::tuple<>,
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

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), std::make_tuple()))
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
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2), decltype(s3), decltype(s4)>,
        std::tuple<>,
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

    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), std::make_tuple()))
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
    /**
     * @brief Test coiteration for dense and hashed formats when function is conjunctive.
     *
     * A conjunctive merge requires coiterating over a dense and hashed format. This
     * test checks that the coiteration is done correctly. The test proceeds as follows:
     *
     * - If one of the levels is unordered (e.g. hashed, or singleton), then the
     * coiteration is done by iterating over the ordered level, and then looking up
     * the corresponding value in the unordered level.
     * - The coiteration stops when the end of the ordered (i.e. dense) level is reached.
     *
     * This test checks that the lookup is done correctly.
     *
     * For example, the following would not compile:
     *
     * auto fn2 = [](std::tuple<bool, bool> t) constexpr { return (std::get<0>(t) ||
     * std::get<1>(t)); }; xsparse::level_capabilities::Coiterate<
     * xsparse::util::LambdaWrapper<decltype(fn2)>::template apply,
     * decltype(fn2),
     * uintptr_t,
     * uintptr_t,
     * std::tuple<decltype(dense_level), decltype(hash_level)>,
     * std::tuple<>>
     * coiter_incorrect(fn2, dense_level, hash_level);
     */
    constexpr uint8_t ZERO = 0;

    std::unordered_map<uintptr_t, uintptr_t> const umap1{ { 0, 1 }, { 2, 5 }, { 1, 2 } };
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

    xsparse::level_capabilities::Coiterate<
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(dense_level), decltype(hash_level)>,
        std::tuple<>,
        std::tuple<>>
        coiter(fn, dense_level, hash_level);

    // define iteration helper through dense and hashed level
    auto it_helper1 = dense_level.iter_helper(std::make_tuple(), ZERO);

    // initialize iterators for dense and hashed level
    auto it1 = it_helper1.begin();
    auto end1 = it_helper1.end();

    // when co-iterating over levels that are unordered (i.e. hashed), then we use locate to
    // check if the index exists in the hashed level. If not, then we skip it.
    for (auto const [ik, pk_tuple] : coiter.coiter_helper(std::make_tuple(), std::make_tuple()))
    {
        // get the index and pointer from the levels involved in co-iteration
        auto [i1, p1] = *it1;

        // should only iterate over the ordered dense level
        uintptr_t l = i1;
        CHECK(ik == l);

        // check that the dense level has not reached the end
        if (it1 != end1)
        {
            // if both levels have a non-zero value at index "i1", then those values
            // should be present in the co-iterated tuple
            if (i1 == l)
            {
                CHECK(p1 == std::get<0>(pk_tuple).value());
            }

            // check that the pk_tuple has the correct values when hash level has a value
            auto hash_level_pk = hash_level.locate(0, i1);
            if (hash_level_pk.has_value())
            {
                CHECK(hash_level_pk == std::get<1>(pk_tuple).value());
            }
            else
            {
                CHECK(hash_level_pk.has_value() == false);
            }

            // increment through the dense level always
            ++it1;
        }
    }

    // check that the dense level should've reached its end
    CHECK((it1 == end1) == true);
}


TEST_CASE("Coiteration-Nested-Levels")
{
    // consider two nested compressed levels, where we coiterate over the dense and compressed levels
    constexpr uintptr_t SIZE1 = 3;
    constexpr uintptr_t SIZE2 = 100;
    constexpr uint8_t ZERO = 0;
    std::vector<uintptr_t> const pos{ 0, 2, 5, 9 };
    std::vector<uintptr_t> const crd{ 20, 50, 30, 40, 70, 10, 60, 80, 90 };

    // the first CSR "tensor"
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d1{ SIZE1 };
    xsparse::levels::compressed<std::tuple<decltype(d1)>,
                                uintptr_t,
                                uintptr_t,
                                xsparse::util::container_traits<std::vector, std::set, std::map>,
                                xsparse::level_properties<true, false, false, false, true>>
        s1{ SIZE2, pos, crd };

    // the second CSR "tensor"
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> d2{ SIZE1 };
    xsparse::levels::compressed<std::tuple<decltype(d2)>,
                                uintptr_t,
                                uintptr_t,
                                xsparse::util::container_traits<std::vector, std::set, std::map>,
                                xsparse::level_properties<true, false, false, false, true>>
        s2{ SIZE2, pos, crd };

    // define a conjunctive function
    auto fn = [](std::tuple<bool, bool> t) constexpr { return (std::get<0>(t) && std::get<1>(t)); };

    // define two coiteration objects that coiterate over the dense and compressed levels respectively
    xsparse::level_capabilities::Coiterate<
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(s1), decltype(s2)>,
        std::tuple<>,
        std::tuple<>>
        coiter_compressed(fn, s1, s2);
     xsparse::level_capabilities::Coiterate<
        xsparse::util::LambdaWrapper<decltype(fn)>::template apply,
        decltype(fn),
        uintptr_t,
        uintptr_t,
        std::tuple<decltype(d1), decltype(d2)>,
        std::tuple<>,
        std::tuple<>>
        coiter_dense(fn, d1, d2);

    auto it_helper1 = d1.iter_helper(std::make_tuple(), ZERO);
    auto it1 = it_helper1.begin();
    auto it_helper2 = d2.iter_helper(std::make_tuple(), ZERO);
    auto it2 = it_helper2.begin();

    auto end1 = it_helper1.end();
    auto end2 = it_helper2.end();

    // when co-iterating over levels that are unordered (i.e. hashed), then we use locate to
    // check if the index exists in the hashed level. If not, then we skip it.
    for (auto const [ik, pk_tuple] : coiter_dense.coiter_helper(std::make_tuple(), std::make_tuple(ZERO, ZERO)))
    {
         // get the index and pointer from the outer-most level involved in co-iteration
        auto [i1, p1] = *it1;
        auto [i2, p2] = *it2;

        // the index should be the same for both levels and tracked via the co-iterator
        CHECK(ik == i1);
        CHECK(ik == i2);
        CHECK(std::get<0>(pk_tuple).value() == p1);
        CHECK(std::get<1>(pk_tuple).value() == p2);

        // use these to define the inner-most iterator
        auto it_helper_inner1 = s1.iter_helper(i1, p1);
        auto it_helper_inner2 = s2.iter_helper(i2, p2);
        auto it1_inner = it_helper_inner1.begin();
        auto it2_inner = it_helper_inner2.begin();
        auto end1_inner = it_helper_inner1.end();
        auto end2_inner = it_helper_inner2.end();

        // co-iterate over the inner-most compressed level now
        for (auto const [cik, cpk_tuple] : coiter_compressed.coiter_helper(std::make_tuple(ik), pk_tuple))
        {
            auto [ci1, cp1] = *it1_inner;
            auto [ci2, cp2] = *it2_inner;
            uintptr_t l = std::min(ci1, ci2);

            if (ci1 == l)
            {
                CHECK(cp1 == std::get<0>(cpk_tuple).value());
                ++it1_inner;
            }
            if (ci2 == l)
            {
                CHECK(cp2 == std::get<1>(cpk_tuple).value());
                ++it2_inner;
            }
        }

        // increment both dense iterators
        ++it1;
        ++it2;
    }

    // check that the dense levelS should've reached its end
    CHECK((it1 == end1) == true);
    CHECK((it2 == end2) == true);
}
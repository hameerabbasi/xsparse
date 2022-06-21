#include <doctest/doctest.h>
#include <xsparse/levels/hashed.hpp>

TEST_CASE("Hashed-BaseCase")
{
    constexpr uintptr_t SIZE = 2;

    std::unordered_map<uintptr_t, uintptr_t> const umap1{ { 1, 2 }, { 3, 4 }, { 5, 6 } };
    std::unordered_map<uintptr_t, uintptr_t> const umap2{ { 7, 8 }, { 9, 10 }, { 11, 12 } };

    std::vector<std::unordered_map<uintptr_t, uintptr_t>> const crd{ umap1, umap2 };

    xsparse::levels::hashed<std::tuple<>,
                            uintptr_t,
                            uintptr_t,
                            std::vector<std::unordered_map<uintptr_t, uintptr_t>>>
        h{ SIZE, crd };

    for (uintptr_t l1 = 0; l1 < SIZE; ++l1)
    {
        auto i_helper = h.iter_helper(l1);
        for (auto it = i_helper.begin(); it != i_helper.end(); ++it)
        {
            CHECK(crd[l1].at(it->first) == it->second);
        }
    }
}

TEST_CASE("Hashed-Locate")
{
    /*
        if (auto a = h.locate(uintptr_t pkm1, uintptr_t ik))
        {
            yield *a; ------> gives pk
        }
    */
}

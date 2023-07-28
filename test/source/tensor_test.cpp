#include <doctest/doctest.h>

#include <xsparse/version.h>
#include <xsparse/tensor.hpp>

#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/singleton.hpp>
#include <xsparse/levels/hashed.hpp>


TEST_CASE("Singleton-level-Tensor")
{
    // construct a single level
    constexpr uintptr_t SIZE = 20;

    // iks
    std::vector<uintptr_t> const crd1{ 0, 1, 0, 1, 0, 3, 4 };

    // values resulting from the pk pointing into data1
    std::vector<std::double_t> const data1{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };

    xsparse::levels::singleton<std::tuple<>, uintptr_t, uintptr_t> s{ SIZE, crd1 };

    // Define a singleton level as tensor
    xsparse::Tensor<std::double_t, std::tuple<decltype(s)>, decltype(data1)> t1(s, data1);

    CHECK(t1.ndim() == 1);
    CHECK(t1.shape() == std::make_tuple(SIZE));
    CHECK(std::get<0>(t1.get_levels()).size() == SIZE);
    CHECK(std::is_same_v<decltype(t1)::dtype, std::double_t>);
}


TEST_CASE("Multiple-levels-Tensor")
{
    constexpr uintptr_t SIZE1 = 3;

    std::unordered_map<uintptr_t, uintptr_t> const umap1{ { 5, 2 }, { 6, 1 }, { 4, 0 } };
    std::unordered_map<uintptr_t, uintptr_t> const umap2{ { 2, 5 } };
    std::unordered_map<uintptr_t, uintptr_t> const umap3{ { 3, 3 }, { 1, 4 }, { 0, 6 } };
    std::vector<std::unordered_map<uintptr_t, uintptr_t>> const crd{ umap1, umap2, umap3 };

    // construct a 3D tensor of dense, dense, hashed
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> s1{ 5 };
    xsparse::levels::dense<std::tuple<decltype(s1)>, uintptr_t, uintptr_t> s2{ 6 };
    xsparse::levels::hashed<std::tuple<decltype(s2)>, uintptr_t, uintptr_t> h{ SIZE1, crd };
    xsparse::Tensor<std::int8_t, std::tuple<decltype(s1), decltype(s2), decltype(h)>> t1(
        s1, s2, h);

    CHECK(t1.ndim() == 3);
    CHECK(t1.shape() == std::make_tuple(5, 6, SIZE1));
    CHECK(std::get<2>(t1.get_levels()).size() == SIZE1);
    CHECK(std::is_same_v<decltype(t1)::dtype, std::int8_t>);
}

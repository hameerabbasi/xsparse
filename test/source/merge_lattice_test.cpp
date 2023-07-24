#include <doctest/doctest.h>

#include <xsparse/tensor.hpp>
#include <xsparse/merge_lattice.hpp>
#include <xsparse/levels/compressed.hpp>
#include <xsparse/levels/dense.hpp>
#include <xsparse/levels/singleton.hpp>
#include <xsparse/levels/hashed.hpp>

#include <xsparse/util/container_traits.hpp>
#include <xsparse/level_properties.hpp>


TEST_CASE("MergeLattice-Dense-Matrix-Elementwise-Multiplication")
{
    /**
     * @brief Test simple matrix element-wise multiplication.
     *
     * A_ij * B_ij = C_ij
     *
     * - Tensors would be (A, B) with shapes (N, M)
     * - Indices would be vectors (0, 1), (0, 1)
     *
     * Coiteration should start at A_00 and B_00, and end at A_NM and B_NM.
     * We would first coiterate over index 0 and if both A_i and B_i are non-zero,
     * we would continue coiterating over index 1 and if both A_ij and B_ij are non-zero,
     * then we can de-reference the value.
     *
     * This value would be stored in a new tensor C.
     */
    constexpr uintptr_t SIZE0 = 5;
    constexpr uintptr_t SIZE1 = 3;

    // define two dense tensors, `A` and `B` of shape (SIZE0, SIZE1)
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> ai{ SIZE0 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> bi{ SIZE0 };
    xsparse::levels::dense<std::tuple<decltype(ai)>, uintptr_t, uintptr_t> aij{ SIZE1 };
    xsparse::levels::dense<std::tuple<decltype(bi)>, uintptr_t, uintptr_t> bij{ SIZE1 };
    xsparse::Tensor<int, std::tuple<decltype(aij), decltype(ai)>> A{ aij, ai };
    xsparse::Tensor<int, std::tuple<decltype(bij), decltype(bi)>> B{ bij, bi };

    // now define a merge lattice iterator
    auto indices = std::make_tuple(std::vector<std::size_t>(0, 1), std::vector<std::size_t>(0, 1));
    auto fn = [](std::tuple<bool, bool> t) constexpr { return (std::get<0>(t) && std::get<1>(t)); };
    auto merge_lattice = xsparse::MergeLattice<decltype(fn),
                                               std::double_t,
                                               decltype(indices),
                                               std::tuple<decltype(A), decltype(B)>>(A, B);

    // now iterate over the merge lattice
    // for (auto const [ik, pk_tuple] : merge_lattice.iter())
    // {
    //     auto [i1, p1] = std::get<0>(pk_tuple);
    //     auto [i2, p2] = std::get<1>(pk_tuple);
    //     CHECK(ik == std::min(i1, i2));
    //     if (i1 == ik)
    //     {
    //         CHECK(p1 == ai.value());
    //     }
    //     if (i2 == ik)
    //     {
    //         CHECK(p2 == bi.value());
    //     }
    // }

    // CHECK(true);
}

TEST_CASE("MergeLattice-Sparse-MatrixAddition-with-Vector-Multiplication")
{
    /**
     * @brief Test matrix addition with vector multiplication.
     *
     * (A_ij + B_ij) @ D_i = C_i
     *
     * - Tensors would be (A, B, D) with shapes (N, M), (N, M) and (M,)
     * - Indices would be vectors (0, 1), (0, 1), (0,)
     *
     * Coiteration should start at A_00, B_00, D_0, and end at A_NM, B_NM, D_M.
     * We would first coiterate over index 0 and if A_i, B_i and D_i are non-zero,
     * we would continue coiterating over index 1 for both A_ij and B_ij. If
     * A_ij and B_ij are non-zero, then we can de-reference the value.
     *
     * Otherwise, we would just continue coiterating over index 0 for A_i, B_i
     * and D_i.
     *
     * This value would be stored in a new tensor C.
     */
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> A{ 5 };
    xsparse::levels::dense<std::tuple<>, uintptr_t, uintptr_t> B{ 5 };

    CHECK(true);
}
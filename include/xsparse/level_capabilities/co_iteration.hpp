#ifndef XSPARSE_CO_ITERATION_HPP
#define XSPARSE_CO_ITERATION_HPP

#include "../levels/compressed.hpp"
#include <vector>

namespace xsparse::level_capabilities
{
    template <class T, class U, class IK, class PK>
    T merge(T t, U u)
    {
        uintptr_t SIZE = 100;
        std::vector<PK> pos;
        std::vector<IK> crd;
        auto cur = -1;
        int l1 = 0;
        for (auto const [i, p] : t.iter_helper(std::make_tuple(), 0))
        {
            for (auto const [i1, p1] : u.iter_helper(std::make_tuple(i), p))
            {
                crd.push_back(i1);
            }
            if (i != cur)
            {
                pos.push_back(l1);
                for (auto k = 0; k < i - cur - 1; ++k)
                {
                    pos.push_back(l1);
                }
                cur = i;
            }
            ++l1;
        }
        pos.push_back(l1);
        T c{ SIZE, pos, crd };
        return c;
    }
}

#endif

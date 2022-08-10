//
// Created by Hameer Abbasi on 22.01.21.
//

#ifndef XSPARSE_LEVEL_PROPERTIES_HPP
#define XSPARSE_LEVEL_PROPERTIES_HPP

namespace xsparse
{
    template <bool Full, bool Ordered, bool Unique, bool Branchless, bool Compact>
    class level_properties;

    template <bool Full, bool Ordered, bool Unique, bool Branchless, bool Compact>
    class level_properties
    {
        constexpr static bool is_full = Full;
        constexpr static bool is_ordered = Ordered;
        constexpr static bool is_unique = Unique;
        constexpr static bool is_branchless = Branchless;
        constexpr static bool is_compact = Compact;
    };
}

#endif  // XSPARSE_LEVEL_PROPERTIES_HPP

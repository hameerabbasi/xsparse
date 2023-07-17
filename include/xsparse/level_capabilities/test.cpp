#include <iostream>
#include <tuple>
#include <type_traits>

// Helper type traits to check if a Level is ordered or unordered
template <typename T>
struct IsOrdered {
    static constexpr bool value = T::is_ordered;
};

// Step 1: Recursive helper function to construct the tuple
template <std::size_t N, typename... Ts>
constexpr auto constructTupleHelper(bool bool_condition) {
    if constexpr (N == 0) {
        return std::tuple<>{};
    } else {
        if constexpr (N == 1) {
            return std::make_tuple(bool_condition);
        } else {
            // Get the first level type from the parameter pack
            using FirstLevel = typename std::tuple_element<sizeof...(Ts) - N, std::tuple<Ts...>>::type;

            // Add the appropriate value to the tuple based on whether the level is ordered or unordered
            if constexpr (IsOrdered<FirstLevel>::value) {
                return std::tuple_cat(constructTupleHelper<N - 1, Ts...>(bool_condition), std::make_tuple(false));
            } else {
                return std::tuple_cat(constructTupleHelper<N - 1, Ts...>(bool_condition), std::make_tuple(true));
            }
        }
    }
}

// Functor that wraps the target function as a callable object
struct TargetFunction {
    constexpr bool operator()(std::tuple<bool, bool, bool> t) const {
        return ((std::get<0>(t) && std::get<2>(t)) && std::get<1>(t));
    }
};

// Step 2: Main helper function
template <class F, std::size_t N, typename... Levels>
constexpr void check_helper() {
    constexpr bool result = F{}(constructTupleHelper<N, Levels...>(true));
    static_assert(result == false);
}

class Level {
public:
    static const bool is_ordered = true;
};

class UnorderedLevel {
public:
    static const bool is_ordered = false;
};

int main() {
    constexpr Level A = Level();
    constexpr UnorderedLevel C = UnorderedLevel();
    constexpr UnorderedLevel B = UnorderedLevel();

    // Usage example:
    constexpr std::size_t N = 3; // Change 'N' to the desired number of elements

    // Call check_helper with the target function 'TargetFunction' and the levels as template arguments
    check_helper<TargetFunction, N, Level, UnorderedLevel, Level>();

    return 0;
}

#include <iostream>
#include <tuple>
#include <type_traits>

//  * TODO: need to concatenate all the level values before this
//  * at level == 0: static_assert and template specialization instead of tuple_cat
//  * because it will compile that specific function call for that specific set of boolean arguments
//  * -> static_assert here.
//  * 

//  * I want a function that recursively constructs a tuple of booleans of size LevelsSize.
//  * The function recursively branches depending on a level property of the level at index I.
//  * 
//  * For example, if `std::get<I>(Levels).LevelProperties::is_ordered == false`, then the function
//  * will branch into two recursive calls, one with `level_bool == true` and one with `level_bool == false`.
//  * 
//  * Once the full tuple of booleans is constructed, it is passed to the function `F`, and the result
//  * is checked to see if the levels are coiterable.

// For example, say we have Levels = (A, B, C, D) with B and D unordered. The recursion would proceed as follows:

// 1. [On index 0 corresponding to `A`] construct_boolean_args<F, std::tuple<bool>(false), 4, 0>() 
// 2. [On index 1 corresponding to `B`, which branches]
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, false), 4, 0>()
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, true), 4, 0>()
// 3. [On index 2 corresponding to `C`]
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, false, false), 4, 0>()
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, true, false), 4, 0>()
// 4. [On index 2 corresponding to `D`, which branches again]
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, false, false, false), 4, 0>()
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, false, false, true), 4, 0>()
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, true, false, false), 4, 0>()
//     - construct_boolean_args<F, std::tuple<bool, bool>(false, true, false, true), 4, 0>()

// At the end, we will have four function evaluations of `F` corresponding to the four tuples of booleans
// constructed above. If any of the function evaluations return true, then the levels are not coiterable.

// tparam F: A function object that takes in a tuple of booleans and returns a boolean.
// tparam LevelsSize: The size of the tuple of levels.
// tparam I: The current index of the level we are checking.
// tparam PrevArgs: A tuple of booleans that is the result of the previous recursive calls.
// tparam Levels: A tuple of levels.

// template <class F, std::size_t LevelsSize, std::size_t I, typename... PrevArgs, class Level, class... Levels>
// constexpr auto validate_boolean_args()
// {
//     static_assert(true);
// }

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

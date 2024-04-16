#include <iostream>
#include <assert.h>

#include "karnaugh.h"

#define LOG(x) if (ENABLE_DEBUG_LOGS) std::cout << x;

#define TEST(void_fn) \
    void_fn(); \
    LOG("TEST COMPLETED: " << #void_fn << std::endl);

using namespace karnaugh;

std::set<literal> make_literals(
    const int a_variable_count
)
{
    std::set<literal> l_result;

    const int l_literal_count = 2 * a_variable_count;

    for (int i = 0; i < l_literal_count; i++)
        l_result.insert(i);

    return l_result;
    
}

template<typename T>
std::set<const T*> const_pointers(
    const std::set<T>& a_set
)
{
    std::set<const T*> l_result;

    for (const T& l_element : a_set)
        l_result.insert(&l_element);

    return l_result;
    
}

////////////////////////////////////////////
//////////////// UNIT TESTS ////////////////
////////////////////////////////////////////
#pragma region UNIT TESTS

void test_small_generalization_0(

)
{
    std::set<zero> l_dissatisfying_inputs =
    {
        { 0, 1, 1 },
        { 0, 1, 0 },
        { 0, 0, 0 }
    };

    std::set<one> l_satisfying_inputs =
    {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 0, 0, 1 }
    };
    
    karnaugh::tree l_tree(
        make_literals(3),
        coverage {
            .m_zeroes = const_pointers(l_dissatisfying_inputs),
            .m_ones = const_pointers(l_satisfying_inputs)
        }
    );

    assert(l_tree.evaluate({0, 0, 0}) == false);
    assert(l_tree.evaluate({0, 0, 1}) == true);
    assert(l_tree.evaluate({0, 1, 0}) == false);
    assert(l_tree.evaluate({0, 1, 1}) == false);
    assert(l_tree.evaluate({1, 0, 0}) == true);
    assert(l_tree.evaluate({1, 0, 1}) == true);
    assert(l_tree.evaluate({1, 1, 0}) == true);
    assert(l_tree.evaluate({1, 1, 1}) == true);

}

void unit_test_main(

)
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_small_generalization_0);
    
}

#pragma endregion

int main(

)
{
    unit_test_main();
}

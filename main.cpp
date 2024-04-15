#include <iostream>
#include <assert.h>

#include "input.h"
#include "logic.h"

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

void test_small_generalization_0(

)
{
    std::set<dissatisfying_input> l_dissatisfying_inputs =
    {
        { 0, 1, 1 },
        { 0, 1, 0 },
        { 0, 0, 0 }
    };

    std::set<satisfying_input> l_satisfying_inputs =
    {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 0, 0, 1 }
    };
    
    karnaugh::model l_model(
        make_literals(3),
        const_pointers(l_dissatisfying_inputs),
        const_pointers(l_satisfying_inputs)
    );

    assert(l_model.evaluate({0, 0, 0}) == false);
    assert(l_model.evaluate({0, 0, 1}) == true);
    assert(l_model.evaluate({0, 1, 0}) == false);
    assert(l_model.evaluate({0, 1, 1}) == false);
    assert(l_model.evaluate({1, 0, 0}) == true);
    assert(l_model.evaluate({1, 0, 1}) == true);
    assert(l_model.evaluate({1, 1, 0}) == true);
    assert(l_model.evaluate({1, 1, 1}) == true);

}

void unit_test_main(

)
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_small_generalization_0);
    
}

int main(

)
{
    unit_test_main();
}

#include <iostream>
#include <assert.h>

#include "karnaugh.h"

#define LOG(x) if (ENABLE_DEBUG_LOGS) std::cout << x;

#define TEST(void_fn) \
    void_fn(); \
    LOG("TEST COMPLETED: " << #void_fn << std::endl);

using namespace karnaugh;

////////////////////////////////////////////
//////////////// UNIT TESTS ////////////////
////////////////////////////////////////////
#pragma region UNIT TESTS

void test_small_generalization_0(

)
{
    std::set<zero> l_zeroes =
    {
        { 0, 1, 1 },
        { 0, 1, 0 },
        { 0, 0, 0 }
    };

    std::set<one> l_ones =
    {
        { 1, 1, 1 },
        { 1, 0, 1 },
        { 0, 0, 1 }
    };
    
    karnaugh::tree l_tree(
        make_literals(3),
        make_coverage(l_zeroes, l_ones)
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

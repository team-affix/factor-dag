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

void test_input_construction(

)
{
    input l_input = { 0, 1, 1, 0, 0, 1 };

    assert(l_input[0] == 0);
    assert(l_input[1] == 1);
    assert(l_input[2] == 1);
    assert(l_input[3] == 0);
    assert(l_input[4] == 0);
    assert(l_input[5] == 1);
    
}

void test_make_coverage(

)
{
    std::set<zero> l_zeroes =
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 1}
    };

    std::set<one> l_ones =
    {
        {1, 0, 0, 1},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 1}
    };

    coverage l_coverage = make_coverage(l_zeroes, l_ones);

    /// Quick sanity check on sizes of results.
    assert(l_coverage.m_zeroes.size() == l_zeroes.size());
    assert(l_coverage.m_ones.size() == l_ones.size());

    /// Loop through each initial vector,
    ///     making sure the pointers to the
    ///     elements are contained in the
    ///     resulting vectors.

    for (const zero& l_zero : l_zeroes)
        assert(l_coverage.m_zeroes.contains(&l_zero));

    for (const one& l_one : l_ones)
        assert(l_coverage.m_ones.contains(&l_one));

}

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

void test_small_generalization_1(

)
{
    std::set<zero> l_zeroes =
    {
        {0, 1, 0, 0},
        {0, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 1, 1, 1},
        {0, 0, 1, 0},
        {0, 1, 1, 0}
    };

    std::set<one> l_ones =
    {
        {1, 0, 0, 0},
        {1, 1, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 1, 1},
        {1, 1, 1, 0},
        {1, 0, 1, 0}
    };

    karnaugh::tree l_tree(
        make_literals(4),
        make_coverage(l_zeroes, l_ones)
    );

    assert(l_tree.evaluate({0, 0, 0, 0}) == false);
    assert(l_tree.evaluate({0, 0, 0, 1}) == false);
    assert(l_tree.evaluate({0, 0, 1, 0}) == false);
    assert(l_tree.evaluate({0, 0, 1, 1}) == false);
    assert(l_tree.evaluate({0, 1, 0, 0}) == false);
    assert(l_tree.evaluate({0, 1, 0, 1}) == false);
    assert(l_tree.evaluate({0, 1, 1, 0}) == false);
    assert(l_tree.evaluate({0, 1, 1, 1}) == false);

    assert(l_tree.evaluate({1, 0, 0, 0}) == true);
    assert(l_tree.evaluate({1, 0, 0, 1}) == true);
    assert(l_tree.evaluate({1, 0, 1, 0}) == true);
    assert(l_tree.evaluate({1, 0, 1, 1}) == true);
    assert(l_tree.evaluate({1, 1, 0, 0}) == true);
    assert(l_tree.evaluate({1, 1, 0, 1}) == true);
    assert(l_tree.evaluate({1, 1, 1, 0}) == true);
    assert(l_tree.evaluate({1, 1, 1, 1}) == true);
    
}

void unit_test_main(

)
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_input_construction);
    TEST(test_make_coverage);
    TEST(test_small_generalization_0);
    TEST(test_small_generalization_1);
    
}

#pragma endregion

int main(

)
{
    unit_test_main();
}

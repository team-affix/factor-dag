#include <iostream>
#include <assert.h>
#include <sstream>

#include "include/dag_logic.h"

#define LOG(x) if (ENABLE_DEBUG_LOGS) std::cout << x;

#define TEST(void_fn) \
    void_fn(); \
    LOG("TEST COMPLETED: " << #void_fn << std::endl);

using namespace dag;
using namespace logic;

////////////////////////////////////////////
//////////////// UNIT TESTS ////////////////
////////////////////////////////////////////
#pragma region UNIT TESTS

void test_node_constructor(

)
{
    node l_nodes[2] = {
        {0, ZERO, ZERO},
        {0, ONE, ONE},
    };

    constexpr uint32_t DEPTH = 13;
    const node* NEGATIVE = &l_nodes[0];
    const node* POSITIVE = &l_nodes[1];
    
    node l_node(DEPTH, NEGATIVE, POSITIVE);

    assert(l_node.depth() == DEPTH);
    assert(l_node.negative() == NEGATIVE);
    assert(l_node.positive() == POSITIVE);
    
};

void test_node_less_than_comparison(

)
{
    node l_nodes[4] = {
        {0, ZERO, ZERO},
        {0, ZERO, ZERO},
        {0, ZERO, ZERO},
        {0, ZERO, ZERO}
    };

    node l_node_0(13, &l_nodes[0], &l_nodes[0]);
    node l_node_1(13, &l_nodes[0], &l_nodes[1]);
    node l_node_2(13, &l_nodes[1], &l_nodes[0]);
    node l_node_3(14, &l_nodes[0], &l_nodes[0]);

    assert(l_node_0 < l_node_1);
    assert(l_node_0 < l_node_2);
    assert(l_node_0 < l_node_3);

    assert(l_node_1 < l_node_2);
    assert(l_node_1 < l_node_3);

    assert(l_node_2 < l_node_3);

    assert(!(l_node_0 < l_node_0));

    assert(!(l_node_1 < l_node_0));
    assert(!(l_node_1 < l_node_1));

    assert(!(l_node_2 < l_node_0));
    assert(!(l_node_2 < l_node_1));
    assert(!(l_node_2 < l_node_2));

    assert(!(l_node_3 < l_node_0));
    assert(!(l_node_3 < l_node_1));
    assert(!(l_node_3 < l_node_2));
    assert(!(l_node_3 < l_node_3));
    
}

void test_node_ostream_inserter(

)
{
    node l_c_bar(2, ONE, ZERO);

    node l_b(1, &l_c_bar, ONE);

    std::stringstream l_ss;

    l_ss << (const node*)&l_c_bar;

    assert(l_ss.str() == "[2']");

    l_ss.str("");

    l_ss << (const node*)&l_b;

    assert(l_ss.str() == "([1'][2']+[1])");
    
}

void test_cache_macro(

)
{
    std::map<int, double> l_cache =
    {
        {0, 1.0},
        {2, 5.0},
        {3, 9.1},
        {5, 0.767},
    };

    assert(CACHE(l_cache, 0, -1.0) == 1.0);
    assert(l_cache.size() == 4);
    
    assert(CACHE(l_cache, 1, -2.0) == -2.0);
    assert(l_cache.size() == 5);

    assert(CACHE(l_cache, 2, -3.0) == 5.0);
    assert(l_cache.size() == 5);

    assert(CACHE(l_cache, 3, -4.0) == 9.1);
    assert(l_cache.size() == 5);

    assert(CACHE(l_cache, 4, -5.0) == -5.0);
    assert(l_cache.size() == 6);

    assert(CACHE(l_cache, 5, -6.0) == 0.767);
    assert(l_cache.size() == 6);

}

void test_node_contraction(

)
{
    std::set<node> l_nodes;

    l_nodes.emplace(node(0, ZERO, ZERO));

    assert(l_nodes.size() == 1);
    
    l_nodes.emplace(node(0, ZERO, ONE));

    /// The above nodes are different, therefore
    ///     we do not expect them to contract.
    assert(l_nodes.size() == 2);

    l_nodes.emplace(node(0, ONE, ZERO));
    
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node(0, ZERO, ZERO));

    /// Now, we expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node(0, ZERO, ONE));

    /// We expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node(0, ONE, ZERO));

    /// We expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node(0, ONE, ONE));

    /// We DO NOT expect the nodes to contract.
    assert(l_nodes.size() == 4);

    l_nodes.emplace(node(10, ZERO, ZERO));
    
    /// Since the above node has a different depth,
    ///     it should NOT contract.
    assert(l_nodes.size() == 5);

}

void test_global_node_sink_bind(

)
{
    std::set<node> l_nodes;

    /// Start off bound to nullptr.
    global_node_sink::bind(nullptr);
    
    /// The return value of bind is the PREVIOUSLY bound
    ///     node sink.
    assert(global_node_sink::bind(&l_nodes) == nullptr);

    assert(global_node_sink::bind(nullptr) == &l_nodes);
    
}

void test_global_node_sink_emplace(

)
{
    std::set<node> l_nodes;
    
    global_node_sink::bind(&l_nodes);
    
    /// Here, we are testing the simplification
    ///     done by global node sink.
    assert(global_node_sink::emplace(0, ZERO, ZERO) == ZERO);
    assert(l_nodes.size() == 0);
    assert(global_node_sink::emplace(0, ONE, ONE) == ONE);
    assert(l_nodes.size() == 0);

    /// Now, insert an unsimplifiable node.
    assert(global_node_sink::emplace(0, ONE, ZERO) == &*l_nodes.begin());

    assert(l_nodes.size() == 1);

    /// Insert an equivalent quantity. This should contract
    ///     with what was already inside the set.
    assert(global_node_sink::emplace(0, ONE, ZERO) != nullptr);

    assert(l_nodes.size() == 1);
    
    assert(global_node_sink::emplace(0, ZERO, ONE) != nullptr);

    assert(l_nodes.size() == 2);

    /// Ensure that this node does not contract with others.
    assert(global_node_sink::emplace(1, ONE, ZERO) != nullptr);

    assert(l_nodes.size() == 3);

    /// Test simplification of emplace given different node depths.
    assert(global_node_sink::emplace(2, ZERO, ZERO) == ZERO);
    assert(l_nodes.size() == 3);
    assert(global_node_sink::emplace(3, ZERO, ZERO) == ZERO);
    assert(l_nodes.size() == 3);

}

void test_literal(

)
{
    std::set<node> l_a_bar_nodes;
    std::set<node> l_a_nodes;
    std::set<node> l_b_bar_nodes;

    /// Bind the GNS.
    global_node_sink::bind(&l_a_bar_nodes);

    /// Construct the literal a'.
    const node* l_a_bar = literal(0, false);

    assert(l_a_bar_nodes.size() == 1);

    /// Since A is the first variable,
    ///     we can interrogate the source
    ///     vertex as it is the A node.
    assert(l_a_bar->depth() == 0);
    assert(l_a_bar->negative() == ONE);
    assert(l_a_bar->positive() == ZERO);
    
    /// Bind to a new set, since we are
    ///     beginning to build a new DAG.
    global_node_sink::bind(&l_a_nodes);
    
    const node* l_a = literal(0, true);

    assert(l_a_nodes.size() == 1);

    assert(l_a->depth() == 0);
    assert(l_a->negative() == ZERO);
    assert(l_a->positive() == ONE);

    /// Once again, bind to new set.
    ///     building a new DAG for b'.
    global_node_sink::bind(&l_b_bar_nodes);

    const node* l_b_bar = literal(1, false);
    
    assert(l_b_bar_nodes.size() == 1);

    // Ensure that the B node only has a negative edge.
    assert(l_b_bar->depth() == 1);
    assert(l_b_bar->negative() == ONE);
    assert(l_b_bar->positive() == ZERO);
    
}

void test_dag_logic_padding(

)
{
    std::set<node> l_nodes;

    global_node_sink::bind(&l_nodes);

    assert(padding<const node*>(false) == ZERO);

    assert(l_nodes.size() == 0);

    assert(padding<const node*>(true) == ONE);

    assert(l_nodes.size() == 0);
    
}

void test_dag_logic_join(

)
{
    std::set<node> l_input_nodes;
    std::set<node> l_result_0_nodes;
    std::set<node> l_result_1_nodes;
    std::set<node> l_result_2_nodes;
    std::set<node> l_result_3_nodes;
    std::set<node> l_result_4_nodes;
    std::set<node> l_result_5_nodes;

    global_node_sink::bind(&l_input_nodes);

    const node* l_a_bar = literal(0, false);
    const node* l_a = literal(0, true);
    const node* l_b_bar = literal(1, false);
    const node* l_b = literal(1, true);
    const node* l_c_bar = literal(2, false);
    const node* l_c = literal(2, true);

    global_node_sink::bind(&l_result_0_nodes);

    /// Disjoin two opposite quantities.
    const node* l_disjunction_0 = disjoin(l_a_bar, l_a);

    /// Nodes size should by zero, since
    ///     the node would have been simplified
    ///     by node::sink before insertion.
    assert(l_result_0_nodes.size() == 0);

    assert(l_disjunction_0 == ONE);

    const node* l_conjunction_0 = conjoin(l_a_bar, l_a);

    assert(l_result_0_nodes.size() == 0);

    /// Conjunction of opposites is zero.
    assert(l_conjunction_0 == ZERO);

    global_node_sink::bind(&l_result_1_nodes);

    /// Disjoin two independent quantities.
    const node* l_disjunction_1 = disjoin(l_a_bar, l_b_bar);
    
    assert(l_result_1_nodes.size() == 1);

    assert(l_disjunction_1->depth() == 0);
    assert(l_disjunction_1->negative() == ONE);
    
    assert(l_disjunction_1->positive()->depth() == 1);
    assert(l_disjunction_1->positive()->negative() == ONE);
    assert(l_disjunction_1->positive()->positive() == ZERO);

    /// Conjoin the independent quantities.
    const node* l_conjunction_1 = conjoin(l_a_bar, l_b_bar);

    assert(l_result_1_nodes.size() == 2);

    assert(l_conjunction_1->depth() == 0);
    assert(l_conjunction_1->positive() == ZERO);
    
    assert(l_conjunction_1->negative()->depth() == 1);
    assert(l_conjunction_1->negative()->negative() == ONE);
    assert(l_conjunction_1->negative()->positive() == ZERO);

    global_node_sink::bind(&l_result_2_nodes);

    const node* l_disjunction_2 = disjoin(l_b, l_a_bar);
    
    assert(l_result_2_nodes.size() == 1);

    assert(l_disjunction_2->depth() == 0);
    assert(l_disjunction_2->negative() == ONE);

    assert(l_disjunction_2->positive()->depth() == 1);
    assert(l_disjunction_2->positive()->negative() == ZERO);
    assert(l_disjunction_2->positive()->positive() == ONE);

    const node* l_conjunction_2 = conjoin(l_b, l_a_bar);

    assert(l_result_2_nodes.size() == 2);

    assert(l_conjunction_2->depth() == 0);
    assert(l_conjunction_2->positive() == ZERO);

    assert(l_conjunction_2->negative()->depth() == 1);
    assert(l_conjunction_2->negative()->negative() == ZERO);
    assert(l_conjunction_2->negative()->positive() == ONE);
    
    global_node_sink::bind(&l_result_3_nodes);

    /// Test disjunction with self.
    const node* l_disjunction_3 = disjoin(l_a, l_a);

    assert(l_disjunction_3->depth() == 0);
    assert(l_disjunction_3->negative() == ZERO);
    assert(l_disjunction_3->positive() == ONE);

    /// Test conjunction with self.
    const node* l_conjunction_3 = conjoin(l_a, l_a);

    assert(l_conjunction_3->depth() == 0);
    assert(l_conjunction_3->negative() == ZERO);
    assert(l_conjunction_3->positive() == ONE);

    global_node_sink::bind(&l_result_4_nodes);

    const node* l_disjunction_4 = disjoin(l_a, l_c);

    assert(l_result_4_nodes.size() == 1);

    assert(l_disjunction_4->depth() == 0);
    assert(l_disjunction_4->positive() == ONE);

    assert(l_disjunction_4->negative()->depth() == 2);
    assert(l_disjunction_4->negative()->negative() == ZERO);
    assert(l_disjunction_4->negative()->positive() == ONE);

    const node* l_conjunction_4 = conjoin(l_a, l_c);

    assert(l_result_4_nodes.size() == 2);

    assert(l_conjunction_4->depth() == 0);
    assert(l_conjunction_4->negative() == ZERO);
    
    assert(l_conjunction_4->positive()->depth() == 2);
    assert(l_conjunction_4->positive()->negative() == ZERO);
    assert(l_conjunction_4->positive()->positive() == ONE);

    global_node_sink::bind(&l_result_5_nodes);

    const node* l_disjunction_5 = disjoin(l_b_bar, l_c);

    assert(l_result_5_nodes.size() == 1);

    assert(l_disjunction_5->depth() == 1);
    assert(l_disjunction_5->negative() == ONE);
    
    assert(l_disjunction_5->positive()->depth() == 2);
    assert(l_disjunction_5->positive()->negative() == ZERO);
    assert(l_disjunction_5->positive()->positive() == ONE);

    const node* l_conjunction_5 = conjoin(l_b_bar, l_c);

    assert(l_result_5_nodes.size() == 2);

    assert(l_conjunction_5->depth() == 1);
    assert(l_conjunction_5->positive() == ZERO);

    assert(l_conjunction_5->negative()->depth() == 2);
    assert(l_conjunction_5->negative()->negative() == ZERO);
    assert(l_conjunction_5->negative()->positive() == ONE);

    /////////////////////////////////
    /// TEST THE CACHE FOR JUNCTION
    /////////////////////////////////

    std::set<node> l_cache_test_nodes;

    global_node_sink::bind(&l_cache_test_nodes);

    std::map<std::set<const node*>, const node*> l_cache;

    const node* l_a_and_c_bar =
        dag::join(
            l_cache,
            ONE,
            ZERO,
            l_a,
            l_c_bar
        );

    /// Only one node actually enters the cache.
    ///     three total calls to the function,
    ///     but the second two are early returns.
    assert(l_cache.size() == 1);

    l_cache.clear();

    const node* l_a_c_bar_or_b =
        dag::join(
            l_cache,
            ZERO,
            ONE,
            l_a_and_c_bar,
            l_b
        );

    assert(l_cache.size() == 2);

    l_cache.clear();
    
    const node* l_a_exnor_b_and_c_bar =
        conjoin(
            disjoin(
                conjoin(
                    l_a_bar, l_b_bar
                ),
                conjoin(
                    l_a, l_b
                )
            ),
            l_c_bar
        );

    const node* l_a_exnor_b_and_c =
        conjoin(
            disjoin(
                conjoin(
                    l_a_bar, l_b_bar
                ),
                conjoin(
                    l_a, l_b
                )
            ),
            l_c
        );

    dag::join(
        l_cache,
        ZERO,
        ONE,
        l_a_exnor_b_and_c_bar,
        l_a_exnor_b_and_c
    );

    assert(l_cache.size() == 4);
    
}

void test_dag_logic_invert(

)
{
    std::set<node> l_input_nodes;
    std::set<node> l_result_nodes;
    
    /// Bind to input node sink.
    global_node_sink::bind(&l_input_nodes);

    /// Construct two input literals.
    const node* l_a_bar = literal(0, false);
    const node* l_a = literal(0, true);
    const node* l_b_bar = literal(1, false);
    const node* l_b = literal(1, true);
    const node* l_c_bar = literal(2, false);
    const node* l_c = literal(2, true);

    /// Bind to output node sink.
    global_node_sink::bind(&l_result_nodes);

    assert(l_a_bar->depth() == 0);
    assert(l_a_bar->negative() == ONE);
    assert(l_a_bar->positive() == ZERO);

    assert(l_b->depth() == 1);
    assert(l_b->negative() == ZERO);
    assert(l_b->positive() == ONE);



    /////////////////////////////////
    /// TEST THE CACHE FOR INVERSION
    /////////////////////////////////

    std::set<node> l_cache_test_nodes;

    global_node_sink::bind(&l_cache_test_nodes);

    std::map<const node*, const node*> l_cache;

    const node* l_a_exnor_b_and_c_bar =
        conjoin(
            disjoin(
                conjoin(
                    l_a_bar, l_b_bar
                ),
                conjoin(
                    l_a, l_b
                )
            ),
            l_c_bar
        );

    dag::invert(l_cache, l_a_exnor_b_and_c_bar);

    assert(l_cache.size() == 4);
    
}

void test_demorgans(

)
{
    std::set<node> l_nodes;

    global_node_sink::bind(&l_nodes);

    const node* l_a = literal(0, true);
    const node* l_b = literal(1, true);

    const node* l_nor = invert(disjoin(l_a, l_b));
    const node* l_and_of_inversions = conjoin(invert(l_a), invert(l_b));

    assert(l_and_of_inversions == l_nor);
    
    const node* l_nand = invert(conjoin(l_a, l_b));
    const node* l_or_of_inversions = disjoin(invert(l_a), invert(l_b));

    assert(l_nand == l_or_of_inversions);

}

void test_composite_function_logic(

)
{
    std::set<node> l_nodes;

    global_node_sink::bind(&l_nodes);

    const node* l_a = literal(0, true);
    const node* l_b = literal(1, true);
    const node* l_c = literal(2, true);
    const node* l_d = literal(3, true);
    const node* l_e = literal(4, true);
    const node* l_f = literal(5, true);

    std::stringstream l_ss;

    l_ss << disjoin(l_a, conjoin(l_b, l_c, l_d), l_e);

    assert(l_ss.str() == "([0']([1'][4]+[1]([2'][4]+[2]([3'][4]+[3])))+[0])");

    l_ss.str("");

    l_ss << conjoin(l_a, l_b, l_c, disjoin(l_d, l_e));

    assert(l_ss.str() == "[0][1][2]([3'][4]+[3])");

    l_ss.str("");

    l_ss << conjoin(l_a, invert(l_b), l_c, invert(conjoin(l_a, l_b)));

    assert(l_ss.str() == "[0][1'][2]");

    l_ss.str("");

    const node* l_exor_a_b =
        exor(
             l_a, l_b
        );

    l_ss << l_exor_a_b;

    assert(l_ss.str() == "([0'][1]+[0][1'])");

    l_ss.str("");

    l_ss << exor(l_a, l_b, l_c);

    assert(l_ss.str() == "([0']([1'][2]+[1][2'])+[0]([1'][2']+[1][2]))");

    l_ss.str("");

    std::list<const node*> l_bs_0 = { l_a, l_b, l_c };
    std::list<const node*> l_bs_1 = { l_d, l_e, l_f };

    l_ss << exnor(l_bs_0, l_bs_1);

    assert(l_ss.str() == "([0']([1']([2'][3'][4'][5']+[2][3'][4'][5])+[1]([2'][3'][4][5']+[2][3'][4][5]))"
                         "+[0]([1']([2'][3][4'][5']+[2][3][4'][5])+[1]([2'][3][4][5']+[2][3][4][5])))"    );

    l_ss.str("");





    /////////////////////////////////
    /// TEST FACTORING PROBLEMS
    /////////////////////////////////

    std::list<const node*> l_bs_2;
    std::list<const node*> l_bs_3;

    const auto l_generate_node = []
    {
        static int i = 0;

        return literal(i++, true);
        
    };
    
    /// Generate two 6-bit strings
    std::generate_n(std::back_inserter(l_bs_2), 6, l_generate_node);
    std::generate_n(std::back_inserter(l_bs_3), 6, l_generate_node);

    const std::list<const node*> l_desired_output = 
        { ONE, ONE, ONE, ZERO, ONE, ONE, ZERO, ZERO, ZERO, ONE, ONE, ZERO };

    l_ss << exnor(multiply(l_bs_2, l_bs_3), l_desired_output);

    assert(l_ss.str() == "[0]([1'][2][3'][4'][5][6][7][8'][9][10'][11]+[1][2'][3][4'][5][6][7'][8][9'][10'][11])");

}

void unit_test_main(

)
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_node_constructor);
    TEST(test_node_less_than_comparison);
    TEST(test_node_ostream_inserter);
    TEST(test_cache_macro);
    TEST(test_node_contraction);
    TEST(test_global_node_sink_bind);
    TEST(test_global_node_sink_emplace);
    TEST(test_literal);
    TEST(test_dag_logic_padding);
    TEST(test_dag_logic_invert);
    TEST(test_dag_logic_join);
    TEST(test_demorgans);
    TEST(test_composite_function_logic);
    
}

#pragma endregion

int main(

)
{
    unit_test_main();
}

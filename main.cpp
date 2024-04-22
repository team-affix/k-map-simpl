#include <iostream>
#include <assert.h>
#include <sstream>

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

void test_utils_pointers(

)
{
    std::set<int> l_ints = {1, 2, 60, 4, 5};

    std::set<const int*> l_pointers = pointers(l_ints);

    assert(l_pointers.size() == l_ints.size());

    for (const int* l_ptr : l_pointers)
        assert(l_ints.contains(*l_ptr));
    
}

void test_utils_filter(

)
{
    std::set<int> l_ints =
    {
        11, 12, 13, 14, 15,
        21, 22, 23, 24, 25,
        31, 32, 33, 34, 35,
        41, 42, 43, 44, 45,
        51, 52, 53, 54, 55,
        61, 62, 63, 64, 65,
        71, 72, 73, 74, 75,
    };

    std::set<int> l_filter_0 = filter(
        l_ints,
        [](
            int a_int
        )
        {
            return a_int < 20;
        }
    );

    std::set<int> l_filter_1 = filter(
        l_ints,
        [](
            int a_int
        )
        {
            return a_int < 30;
        }
    );

    std::set<int> l_filter_2 = filter(
        l_ints,
        [](
            int a_int
        )
        {
            return a_int < 40;
        }
    );
    
    assert(l_filter_0 == std::set({
        11, 12, 13, 14, 15,
    }));

    assert(l_filter_1 == std::set({
        11, 12, 13, 14, 15,
        21, 22, 23, 24, 25,
    }));

    assert(l_filter_2 == std::set({
        11, 12, 13, 14, 15,
        21, 22, 23, 24, 25,
        31, 32, 33, 34, 35,
    }));
    
}

void test_utils_cover(

)
{
    std::set<int> l_ints = {0, 1, 2, 3};

    std::map<double, std::set<int>> l_int_cover = cover(
        l_ints,
        [](
            const int& a_int
        )
        {
            std::set<double> l_result {
                double(a_int),
                double(a_int) / 2,
                double(a_int) / 3,
            };
            
            return l_result;
        }
    );

    assert(l_int_cover.size() == 8);

    assert(l_int_cover[0.0] == std::set({0}));
    assert(l_int_cover[1.0/3.0] == std::set({1}));
    assert(l_int_cover[0.5] == std::set({1}));
    assert(l_int_cover[2.0/3.0] == std::set({2}));
    assert(l_int_cover[1.0] == std::set({1, 2, 3}));
    assert(l_int_cover[1.5] ==  std::set({3}));
    assert(l_int_cover[2.0] == std::set({2}));
    assert(l_int_cover[3.0] == std::set({3}));

}

void test_utils_partition(

)
{
    std::set<int> l_ints = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::map<int, std::set<int>> l_int_cover = partition(
        l_ints,
        [](
            const int& a_int
        )
        {
            return a_int % 3;
        }
    );

    assert(l_int_cover.size() == 3);

    assert(l_int_cover[0] == std::set({0, 3, 6, 9}));
    assert(l_int_cover[1] == std::set({1, 4, 7, 10}));
    assert(l_int_cover[2] == std::set({2, 5, 8}));

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

    l_nodes.emplace(node { .m_left_child = ZERO, .m_right_child = ZERO });

    assert(l_nodes.size() == 1);
    
    l_nodes.emplace(node { .m_left_child = ZERO, .m_right_child = ONE });

    /// The above nodes are different, therefore
    ///     we do not expect them to contract.
    assert(l_nodes.size() == 2);

    l_nodes.emplace(node { .m_left_child = ONE, .m_right_child = ZERO });
    
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node { .m_left_child = ZERO, .m_right_child = ZERO });

    /// Now, we expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node { .m_left_child = ZERO, .m_right_child = ONE });

    /// We expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node { .m_left_child = ONE, .m_right_child = ZERO });

    /// We expect the nodes to contract.
    assert(l_nodes.size() == 3);

    l_nodes.emplace(node { .m_left_child = ONE, .m_right_child = ONE });

    /// We DO NOT expect the nodes to contract.
    assert(l_nodes.size() == 4);

}

void test_global_node_sink(

)
{
    std::set<node> l_nodes;

    /// Start off bound to nullptr.
    global_node_sink::bind(nullptr);
    
    /// The return value of bind is the PREVIOUSLY bound
    ///     node sink.
    assert(global_node_sink::bind(&l_nodes) == nullptr);

    assert(global_node_sink::commit(node{ZERO, ZERO}) == &*l_nodes.begin());

    assert(l_nodes.size() == 1);

    /// Insert an equivalent quantity. This should contract
    ///     with what was already inside the set.
    assert(global_node_sink::commit(node{ZERO, ZERO}) != nullptr);

    assert(l_nodes.size() == 1);
    
    assert(global_node_sink::commit(node{ZERO, ONE}) != nullptr);

    assert(l_nodes.size() == 2);

    assert(global_node_sink::bind(nullptr) == &l_nodes);

    assert(l_nodes.size() == 2);
    
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
    assert(l_a_bar->m_left_child == ONE);
    assert(l_a_bar->m_right_child == ZERO);
    
    /// Bind to a new set, since we are
    ///     beginning to build a new DAG.
    global_node_sink::bind(&l_a_nodes);
    
    const node* l_a = literal(0, true);

    assert(l_a_nodes.size() == 1);

    assert(l_a->m_left_child == ZERO);
    assert(l_a->m_right_child == ONE);

    /// Once again, bind to new set.
    ///     building a new DAG for b'.
    global_node_sink::bind(&l_b_bar_nodes);

    const node* l_b_bar = literal(1, false);
    
    assert(l_b_bar_nodes.size() == 2);

    /// Make sure that both A subchildren are pointing
    ///     to the same location.
    assert(l_b_bar->m_left_child == l_b_bar->m_right_child);

    // Ensure that the B node only has a negative edge.
    assert(l_b_bar->m_left_child->m_left_child == ONE);
    assert(l_b_bar->m_left_child->m_right_child == ZERO);
    
}

void test_literal_invert(

)
{
    std::set<node> l_input_nodes;
    std::set<node> l_result_nodes;
    
    /// Bind to input node sink.
    global_node_sink::bind(&l_input_nodes);

    /// Construct two input literals.
    const node* l_a = literal(0, true);
    const node* l_b_bar = literal(1, false);

    /// Bind to output node sink.
    global_node_sink::bind(&l_result_nodes);

    const node* l_a_bar = invert(l_a);

    assert(l_a_bar->m_left_child == ONE);
    assert(l_a_bar->m_right_child == ZERO);

    const node* l_b = invert(l_b_bar);

    assert(l_b->m_left_child == l_b->m_right_child);

    assert(l_b->m_left_child->m_left_child == ZERO);
    assert(l_b->m_left_child->m_right_child == ONE);
    
}

void test_literal_disjoin(

)
{
    std::set<node> l_input_nodes;
    std::set<node> l_result_0_nodes;
    std::set<node> l_result_1_nodes;
    std::set<node> l_result_2_nodes;

    global_node_sink::bind(&l_input_nodes);

    const node* l_a_bar = literal(0, false);
    const node* l_a = literal(0, true);
    const node* l_b_bar = literal(1, false);
    const node* l_b = literal(1, true);

    global_node_sink::bind(&l_result_0_nodes);

    /// Disjoin two opposite quantities.
    const node* l_disjunction_0 = disjoin(l_a_bar, l_a);

    assert(l_result_0_nodes.size() == 1);

    assert(l_disjunction_0 == ONE);

    global_node_sink::bind(&l_result_1_nodes);

    /// Disjoin two independent quantities.
    const node* l_disjunction_1 = disjoin(l_a_bar, l_b_bar);
    
    assert(l_result_1_nodes.size() == 1);

    assert(l_disjunction_1->m_left_child == ONE);
    assert(l_disjunction_1->m_right_child->m_left_child == ONE);
    assert(l_disjunction_1->m_right_child->m_right_child == ZERO);

    global_node_sink::bind(&l_result_2_nodes);

    const node* l_disjunction_2 = disjoin(l_a_bar, l_b);
    
    assert(l_result_2_nodes.size() == 1);

    assert(l_disjunction_2->m_left_child == ONE);
    assert(l_disjunction_2->m_right_child->m_left_child == ZERO);
    assert(l_disjunction_2->m_right_child->m_right_child == ONE);
    
}

// void test_literal_sign(

// )
// {
//     assert(sign(literal(0)) == 0);
//     assert(sign(literal(1)) == 1);
//     assert(sign(literal(2)) == 0);
//     assert(sign(literal(3)) == 1);
//     assert(sign(literal(47)) == 1);
//     assert(sign(literal(48)) == 0);
// }

// void test_literal_index(

// )
// {
//     assert(index(literal(0)) == 0);
//     assert(index(literal(1)) == 0);
//     assert(index(literal(2)) == 1);
//     assert(index(literal(3)) == 1);
//     assert(index(literal(47)) == 23);
//     assert(index(literal(48)) == 24);
// }

// void test_literal_covers(

// )
// {
//     input l_input = { 0, 1, 1, 0, 1, 0 };

//     assert(covers(literal(0), l_input) == true);
//     assert(covers(literal(1), l_input) == false);
//     assert(covers(literal(2), l_input) == false);
//     assert(covers(literal(3), l_input) == true);
//     assert(covers(literal(4), l_input) == false);
//     assert(covers(literal(5), l_input) == true);
//     assert(covers(literal(6), l_input) == true);
//     assert(covers(literal(7), l_input) == false);
//     assert(covers(literal(8), l_input) == false);
//     assert(covers(literal(9), l_input) == true);
//     assert(covers(literal(10), l_input) == true);
//     assert(covers(literal(11), l_input) == false);
    
// }

// void test_make_literals(

// )
// {
//     constexpr size_t VAR_COUNT = 10;
    
//     std::set<literal> l_literals = make_literals(VAR_COUNT);

//     /// The number of literals generated should
//     ///     always be twice the number of variables.
//     assert(l_literals.size() == 2 * VAR_COUNT);

//     /// Ensure that all expected literals exist
//     ///     within the resultant set.
//     for (literal i = 0; i < l_literals.size(); i++)
//         assert(l_literals.contains(i));
    
// }

// void test_small_generalization_0(

// )
// {
//     constexpr bool ENABLE_DEBUG_LOGS = false;
    
//     std::set<input> l_zeroes =
//     {
//         { 0, 1, 1 },
//         { 0, 1, 0 },
//         { 0, 0, 0 }
//     };

//     std::set<input> l_ones =
//     {
//         { 1, 1, 1 },
//         { 1, 0, 1 },
//         { 0, 0, 1 }
//     };
    
//     tree l_tree = generalize(
//         l_zeroes,
//         l_ones
//     );

//     assert(l_tree({0, 0, 0}) == false);
//     assert(l_tree({0, 0, 1}) == true);
//     assert(l_tree({0, 1, 0}) == false);
//     assert(l_tree({0, 1, 1}) == false);
//     assert(l_tree({1, 0, 0}) == true);
//     assert(l_tree({1, 0, 1}) == true);
//     assert(l_tree({1, 1, 0}) == true);
//     assert(l_tree({1, 1, 1}) == true);

//     LOG(l_tree << std::endl);

//     std::stringstream l_ss;

//     l_ss << l_tree;

//     assert(l_ss.str() == "1+2(5)");
    
// }

// void test_small_generalization_1(

// )
// {
//     constexpr bool ENABLE_DEBUG_LOGS = false;
    
//     std::set<input> l_zeroes =
//     {
//         {0, 1, 0, 0},
//         {0, 0, 0, 1},
//         {0, 1, 0, 1},
//         {0, 1, 1, 1},
//         {0, 0, 1, 0},
//         {0, 1, 1, 0}
//     };

//     std::set<input> l_ones =
//     {
//         {1, 0, 0, 0},
//         {1, 1, 0, 1},
//         {1, 0, 0, 1},
//         {1, 0, 1, 1},
//         {1, 1, 1, 0},
//         {1, 0, 1, 0}
//     };

//     tree l_tree = generalize(
//         l_zeroes,
//         l_ones
//     );

//     assert(l_tree({0, 0, 0, 0}) == false);
//     assert(l_tree({0, 0, 0, 1}) == false);
//     assert(l_tree({0, 0, 1, 0}) == false);
//     assert(l_tree({0, 0, 1, 1}) == false);
//     assert(l_tree({0, 1, 0, 0}) == false);
//     assert(l_tree({0, 1, 0, 1}) == false);
//     assert(l_tree({0, 1, 1, 0}) == false);
//     assert(l_tree({0, 1, 1, 1}) == false);

//     assert(l_tree({1, 0, 0, 0}) == true);
//     assert(l_tree({1, 0, 0, 1}) == true);
//     assert(l_tree({1, 0, 1, 0}) == true);
//     assert(l_tree({1, 0, 1, 1}) == true);
//     assert(l_tree({1, 1, 0, 0}) == true);
//     assert(l_tree({1, 1, 0, 1}) == true);
//     assert(l_tree({1, 1, 1, 0}) == true);
//     assert(l_tree({1, 1, 1, 1}) == true);
    
//     LOG(l_tree << std::endl);

//     std::stringstream l_ss;

//     l_ss << l_tree;

//     assert(l_ss.str() == "1");
    
// }

// void test_small_generalization_2(

// )
// {
//     constexpr bool ENABLE_DEBUG_LOGS = false;
    
//     std::set<input> l_zeroes =
//     {
//         {0, 0, 0, 0},
//         {0, 0, 0, 1},
//         {0, 1, 0, 1},
//         {0, 1, 1, 1},
//         {0, 1, 1, 0},
//         {1, 1, 1, 1},
//     };

//     std::set<input> l_ones =
//     {
//         {0, 0, 1, 1},
//         {0, 1, 0, 0},
//         {1, 1, 0, 1},
//         {1, 1, 1, 0},
//         {1, 0, 0, 1},
//         {1, 0, 1, 1},
//         {1, 0, 1, 0},
//     };

//     tree l_tree = generalize(
//         l_zeroes,
//         l_ones
//     );

//     assert(l_tree({0, 0, 0, 0}) == false);
//     assert(l_tree({0, 0, 0, 1}) == false);
//     assert(l_tree({0, 0, 1, 0}) == true); // Unknown
//     assert(l_tree({0, 0, 1, 1}) == true);
//     assert(l_tree({0, 1, 0, 0}) == true);
//     assert(l_tree({0, 1, 0, 1}) == false);
//     assert(l_tree({0, 1, 1, 0}) == false);
//     assert(l_tree({0, 1, 1, 1}) == false);

//     assert(l_tree({1, 0, 0, 0}) == true); // Unknown
//     assert(l_tree({1, 0, 0, 1}) == true);
//     assert(l_tree({1, 0, 1, 0}) == true);
//     assert(l_tree({1, 0, 1, 1}) == true);
//     assert(l_tree({1, 1, 0, 0}) == true); // Unknown
//     assert(l_tree({1, 1, 0, 1}) == true);
//     assert(l_tree({1, 1, 1, 0}) == true);
//     assert(l_tree({1, 1, 1, 1}) == false);
    
//     LOG(l_tree << std::endl);

//     std::stringstream l_ss;

//     l_ss << l_tree;

//     assert(l_ss.str() == "1(2+4+6)+2(5)+6(3(4))");
    
// }

// void test_small_generalization_3(

// )
// {
//     constexpr bool ENABLE_DEBUG_LOGS = false;

//     std::set<input> l_zeroes =
//     {
//         {0, 1, 1, 0},
//         {1, 1, 0, 0},
//         {1, 0, 0, 0},
//         {1, 0, 0, 1},
//         {1, 0, 1, 0},
//     };

//     std::set<input> l_ones =
//     {
//         {0, 0, 0, 0},
//         {0, 1, 0, 0},
//         {1, 1, 1, 1},
//         {1, 0, 1, 1},
//     };

//     tree l_tree = generalize(
//         l_zeroes,
//         l_ones
//     );

//     assert(l_tree({0, 0, 0, 0}) == true);
//     assert(l_tree({0, 0, 0, 1}) == true); // Unknown
//     assert(l_tree({0, 0, 1, 0}) == true); // Unknown
//     assert(l_tree({0, 0, 1, 1}) == true); // Unknown
//     assert(l_tree({0, 1, 0, 0}) == true);
//     assert(l_tree({0, 1, 0, 1}) == true); // Unknown
//     assert(l_tree({0, 1, 1, 0}) == false);
//     assert(l_tree({0, 1, 1, 1}) == true); // Unknown

//     assert(l_tree({1, 0, 0, 0}) == false);
//     assert(l_tree({1, 0, 0, 1}) == false);
//     assert(l_tree({1, 0, 1, 0}) == false);
//     assert(l_tree({1, 0, 1, 1}) == true);
//     assert(l_tree({1, 1, 0, 0}) == false);
//     assert(l_tree({1, 1, 0, 1}) == true);  // Unknown
//     assert(l_tree({1, 1, 1, 0}) == false); // Unknown
//     assert(l_tree({1, 1, 1, 1}) == true);

//     LOG(l_tree << std::endl);

//     std::stringstream l_ss;

//     l_ss << l_tree;

//     assert(l_ss.str() == "0(2+4)+7(3+5)");
    
// }

void unit_test_main(

)
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_utils_pointers);
    TEST(test_utils_filter);
    TEST(test_utils_cover);
    TEST(test_utils_partition);
    TEST(test_cache_macro);
    TEST(test_node_contraction);
    TEST(test_global_node_sink);
    TEST(test_literal);
    TEST(test_literal_invert);
    TEST(test_literal_disjoin);
    // TEST(test_literal_index);
    // TEST(test_literal_sign);
    // TEST(test_literal_covers);
    // TEST(test_make_literals);
    // TEST(test_small_generalization_0);
    // TEST(test_small_generalization_1);
    // TEST(test_small_generalization_2);
    // TEST(test_small_generalization_3);
    
}

// #pragma endregion

int main(

)
{
    unit_test_main();
}

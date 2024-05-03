#ifndef FACTOR_H
#define FACTOR_H

#include <stdint.h>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ostream>
#include <istream>
#include <functional>
#include <stack>

#include "../digital-logic/include/logic.h"

/// This macro function defines
///     getting a value from cache if key contained,
///     otherwise, computing value and caching it.
#define CACHE(cache, key, value) \
    (cache.contains(key) ? cache[key] : cache[key] = value)

namespace factor
{

    ////////////////////////////////////////////
    ////////////// DATA STRUCTURES /////////////
    ////////////////////////////////////////////
    #pragma region DATA STRUCTURES

    class node
    {
        /// Defines the depth of the factor in the tree.
        uint32_t m_depth;

        /// Defines the subtrees.
        const node* m_negative;
        const node* m_positive;

    public:

        node(
            uint32_t a_depth,
            const node* a_left_child,
            const node* a_right_child
        ) :
            m_depth(a_depth),
            m_negative(a_left_child),
            m_positive(a_right_child)
        {

        }
        
        uint32_t depth(

        ) const
        {
            return m_depth;
        }

        const node* negative(

        ) const
        {
            return m_negative;
        }

        const node* positive(

        ) const
        {
            return m_positive;
        }

        bool operator<(
            const node& a_other
        ) const
        {
            if (m_depth != a_other.m_depth)
                return m_depth < a_other.m_depth;
            
            if (m_negative != a_other.m_negative)
                return m_negative < a_other.m_negative;

            if (m_positive != a_other.m_positive)
                return m_positive < a_other.m_positive;

            return false;
            
        }

    };

    std::ostream& operator<<(
        std::ostream& a_ostream,
        const node* a_node
    );

    std::istream& operator>>(
        std::istream& a_istream,
        const node*& a_node
    );

    /// These are terminal nodes in the factor DAG's.
    inline const node* ONE = reinterpret_cast<const node*>(-1);
    inline const node* ZERO = reinterpret_cast<const node*>(0);

    struct dag
    {
        dag(
            
        )
        {

        }

        /// We disallow shallow copying the object,
        ///     as this would cause the copied
        ///     graph's pointers to dangle.
        dag(
            const dag&
        ) = delete;

        /// For the same reason as above,
        ///     we disallow copy assignment.
        dag& operator=(
            const dag&
        ) = delete;

        size_t size(

        ) const
        {
            return m_nodes.size();
        }

        const node* emplace(
            uint32_t a_depth,
            const node* a_negative_child,
            const node* a_positive_child
        )
        {
            if (a_negative_child == a_positive_child)
                /// If the children are identical, just
                ///     perform the simplification and
                ///     avoid emplacing anything.
                return a_negative_child;

            return &*m_nodes.emplace(
                a_depth,
                a_negative_child,
                a_positive_child
            ).first;
            
        }
        
    private:
        std::set<node> m_nodes;

    };

    #pragma endregion

    ////////////////////////////////////////////
    //////////////// GLOBAL VARS ///////////////
    ////////////////////////////////////////////
    #pragma region GLOBAL VARS

    class global_node_sink
    {
        static dag* s_graph;

    public:
        static void bind(
            dag* a_graph
        )
        {
            s_graph = a_graph;
        }

        static dag* bound(

        )
        {
            return s_graph;
        }
        
    };

    #pragma endregion

    ////////////////////////////////////////////
    //////////////// ALGORITHMS ////////////////
    ////////////////////////////////////////////
    #pragma region ALGORITHMS

    inline const node* literal(
        uint32_t a_variable_index,
        bool a_sign
    )
    {
        return
            global_node_sink::bound()->emplace(
                a_variable_index,
                !a_sign ? ONE : ZERO,
                a_sign ? ONE : ZERO
            );
    }

    inline const node* join(
        std::map<std::set<const node*>, const node*>& a_cache,
        const node* a_ident,
        const node* a_antident,
        const node* a_x,
        const node* a_y
    )
    {
        /// If either operand is a zero,
        ///     return the opposite operand.
        if (a_x == a_ident)
            return a_y;
        if (a_y == a_ident)
            return a_x;

        /// If either operand is 1, just
        ///     return 1.
        if (a_x == a_antident || a_y == a_antident)
            return a_antident;

        /// We need to make variable the
        ///     nodes that we will recur on,
        ///     due to the potential for
        ///     differing node depths.
        const node* l_x_left = a_x->negative();
        const node* l_y_left = a_y->negative();
        const node* l_x_right = a_x->positive();
        const node* l_y_right = a_y->positive();

        /// If the depths differ, we mustn't
        ///     traverse to the children of
        ///     the higher-depth node.
        if (a_x->depth() > a_y->depth())
        {
            l_x_left = a_x;
            l_x_right = a_x;
        }
        else if (a_y->depth() > a_x->depth())
        {
            l_y_left = a_y;
            l_y_right = a_y;
        }

        /// Construct the cache key, which
        ///     should be the sorted pair:
        std::set<const node*> l_key = { a_x, a_y };

        return CACHE(
            a_cache,
            l_key,
            global_node_sink::bound()->emplace(
                std::min(a_x->depth(), a_y->depth()),
                join(a_cache, a_ident, a_antident, l_x_left, l_y_left),
                join(a_cache, a_ident, a_antident, l_x_right, l_y_right)
            )
        );

    }

    inline const node* invert(
        std::map<const node*, const node*>& a_cache,
        const node* a_node
    )
    {
        if (a_node == ZERO)
            return ONE;
        if (a_node == ONE)
            return ZERO;

        /// Query the cache and if it is not
        ///     found, store the computed result.
        return CACHE(
            a_cache,
            a_node,
            global_node_sink::bound()->emplace(
                a_node->depth(),
                invert(a_cache, a_node->negative()),
                invert(a_cache, a_node->positive())
            )
        );

    }

    /// Evaluates the function represented by the
    ///     factor DAG on the argued input.
    inline bool evaluate(
        const node* a_node,
        const std::vector<bool>& a_input
    )
    {
        /// Basic cases for evaluation.
        if (a_node == ZERO)
            return false;
        if (a_node == ONE)
            return true;

        if (a_input[a_node->depth()])
            return evaluate(a_node->positive(), a_input);
        else
            return evaluate(a_node->negative(), a_input);
            
    }

    #pragma endregion

}

namespace logic
{

    ////////////////////////////////////////////
    //////// USER-SPECIALIZED DAG LOGIC ////////
    ////////////////////////////////////////////
    #pragma region USER-SPECIALIZED DAG LOGIC

    template<>
    inline const factor::node* padding(
        bool a_logic_state
    )
    {
        return a_logic_state ? factor::ONE : factor::ZERO;
    }

    template<>
    inline const factor::node* join(
        bool a_identity,
        const factor::node* a_x,
        const factor::node* a_y
    )
    {
        /// Construct the function cache.
        std::map<std::set<const factor::node*>, const factor::node*> l_cache;

        return factor::join(
            l_cache,
            a_identity ? factor::ONE : factor::ZERO,
            a_identity ? factor::ZERO : factor::ONE,
            a_x,
            a_y
        );

    }

    template<>
    inline const factor::node* invert(
        const factor::node* a_node
    )
    {
        /// Construct the function cache.
        std::map<const factor::node*, const factor::node*> l_cache;

        /// Call the overload, supplying the cache.
        return factor::invert(l_cache, a_node);
        
    }

    #pragma endregion
    
}

#endif

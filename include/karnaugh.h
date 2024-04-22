#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ostream>
#include <functional>
#include <stack>

#define ONE (const node*)-1
#define ZERO (const node*)nullptr

#define CACHE(cache, key, value) \
    (cache.contains(key) ? cache[key] : cache[key] = value)

namespace karnaugh
{

    ////////////////////////////////////////////
    //////////// UTILITY FUNCTIONS /////////////
    ////////////////////////////////////////////
    #pragma region UTILITY FUNCTIONS

    /// Constructs a set of pointers given
    ///     a set of any type.
    template<typename T>
    inline std::set<const T*> pointers(
        const std::set<T>& a_vals
    )
    {
        std::set<const T*> l_result;
        
        for (const T& l_val : a_vals)
            l_result.insert(&l_val);

        return l_result;
        
    }

    /// Filters the inputted set by
    ///     the supplied predicate.
    template<typename T, typename FUNCTION>
    inline std::set<T> filter(
        const std::set<T>& a_vals,
        const FUNCTION& a_query
    )
    {
        std::set<T> l_result;

        std::copy_if(
            a_vals.begin(),
            a_vals.end(),
            std::inserter(
                l_result,
                l_result.begin()
            ),
            a_query
        );

        return l_result;
        
    }

    /// Returns a cover (see set theory)
    ///     of an inputted set, grouped
    ///     by the supplied lambda.
    template<typename VALUE, typename FUNCTION>
    inline auto cover(
        const std::set<VALUE>& a_values,
        const FUNCTION& a_grouper
    )
    {
        using KEY =
            std::decay_t<
                decltype(*a_grouper(VALUE()).begin())
            >;
        
        std::map<KEY, std::set<VALUE>> l_result;

        for (const VALUE& l_value : a_values)
        {
            std::set<KEY> l_keys = a_grouper(l_value);

            for (const KEY& l_key : l_keys)
                l_result[l_key].insert(l_value);
            
        }

        return l_result;
        
    }

    /// Returns a partition (see set theory)
    ///     of the inputted set, grouped by
    ///     the supplied lambda.
    template<typename VALUE, typename FUNCTION>
    inline auto partition(
        const std::set<VALUE>& a_values,
        const FUNCTION& a_partitioner
    )
    {
        return cover(
            a_values,
            [&a_partitioner](
                VALUE a_value
            )
            {
                return std::set({ a_partitioner(a_value) });
            }
        );
    }

    #pragma endregion

    ////////////////////////////////////////////
    /////////////// INPUT TYPES ////////////////
    ////////////////////////////////////////////
    #pragma region INPUT TYPES

    typedef std::vector<bool> input;

    // typedef uint32_t literal;

    // inline uint32_t index(
    //     literal a_literal
    // )
    // {
    //     return a_literal >> 1;
    // }
    
    // inline bool sign(
    //     literal a_literal
    // )
    // {
    //     return a_literal % 2;
    // }

    // inline bool covers(
    //     literal a_literal,
    //     const input& a_input
    // )
    // {
    //     return a_input.at(index(a_literal)) == sign(a_literal);
    // }

    // inline std::set<literal> make_literals(
    //     const int a_variable_count
    // )
    // {
    //     std::set<literal> l_result;

    //     const int l_literal_count = 2 * a_variable_count;

    //     for (int i = 0; i < l_literal_count; i++)
    //         l_result.insert(i);

    //     return l_result;
        
    // }

    #pragma endregion

    ////////////////////////////////////////////
    ///////////////// MODELING /////////////////
    ////////////////////////////////////////////
    #pragma region MODELING

    class node
    {
        /// Defines the depth of the node in the tree.
        uint32_t m_depth;

        /// Defines the subtrees.
        const node* m_left_child;
        const node* m_right_child;

    public:

        node(
            uint32_t a_depth,
            const node* a_left_child,
            const node* a_right_child
        ) :
            m_depth(a_depth),
            m_left_child(a_left_child),
            m_right_child(a_right_child)
        {

        }
        
        uint32_t depth(

        ) const
        {
            return m_depth;
        }

        const node* left(

        ) const
        {
            return m_left_child;
        }

        const node* right(

        ) const
        {
            return m_right_child;
        }

        bool operator<(
            const node& a_other
        ) const
        {
            if (m_depth != a_other.m_depth)
                return m_depth < a_other.m_depth;
            
            if (m_left_child != a_other.m_left_child)
                return m_left_child < a_other.m_left_child;

            if (m_right_child != a_other.m_right_child)
                return m_right_child < a_other.m_right_child;

            return false;
            
        }

        class sink
        {
            static std::set<node>* s_nodes;

        public:
            static const node* emplace(
                uint32_t a_depth,
                const node* a_left_child,
                const node* a_right_child
            )
            {
                /// Apply simplification to node.
                if (a_left_child == a_right_child)
                    return a_left_child;

                /// This insertion will contract
                ///     any identical expressions.
                return &*s_nodes->emplace(
                    a_depth, a_left_child, a_right_child
                ).first;

            }

            static std::set<node>* bind(
                std::set<node>* a_nodes
            )
            {
                /// Save the previously bound node sink
                std::set<node>* l_result = s_nodes;

                /// Bind to the new node sink
                s_nodes = a_nodes;

                /// Return the node sink that was unbound.
                return l_result;
                
            }
            
        };
        
    };


    inline const node* literal(
        uint32_t a_variable_index,
        bool a_sign
    )
    {
        return
            node::sink::emplace(
                a_variable_index,
                !a_sign ? ONE : ZERO,
                a_sign ? ONE : ZERO
            );
    }

    inline const node* invert(
        const node* a_node,
        std::map<const node*, const node*>& a_cache
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
            node::sink::emplace(
                a_node->depth(),
                invert(a_node->left(), a_cache),
                invert(a_node->right(), a_cache)
            )
        );

    }

    inline const node* invert(
        const node* a_node
    )
    {
        /// Construct the function cache.
        std::map<const node*, const node*> l_cache;

        /// Call the overload, supplying the cache.
        return invert(a_node, l_cache);
        
    }

    inline const node* disjoin(
        const node* a_x,
        const node* a_y,
        std::map<std::set<const node*>, const node*>& a_cache
    )
    {
        /// If either operand is a zero,
        ///     return the opposite operand.
        if (a_x == ZERO)
            return a_y;
        if (a_y == ZERO)
            return a_x;

        /// If either operand is 1, just
        ///     return 1.
        if (a_x == ONE || a_y == ONE)
            return ONE;

        /// We need to make variable the
        ///     nodes that we will recur on,
        ///     due to the potential for
        ///     differing node depths.
        const node* l_x_left = a_x->left();
        const node* l_y_left = a_y->left();
        const node* l_x_right = a_x->right();
        const node* l_y_right = a_y->right();

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
            node::sink::emplace(
                std::min(a_x->depth(), a_y->depth()),
                disjoin(l_x_left, l_y_left, a_cache),
                disjoin(l_x_right, l_y_right, a_cache)
            )
        );

    }

    inline const node* disjoin(
        const node* a_x,
        const node* a_y
    )
    {
        /// Construct the function cache.
        std::map<std::set<const node*>, const node*> l_cache;

        /// Call the overload, supplying the cache.
        return disjoin(a_x, a_y, l_cache);
        
    }
    
    #pragma endregion
    
}

#endif

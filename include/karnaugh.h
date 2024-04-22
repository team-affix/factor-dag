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

    struct node
    {
        const node* m_left_child;
        const node* m_right_child;

        bool operator<(
            const node& a_other
        ) const
        {
            if (m_left_child != a_other.m_left_child)
                return m_left_child < a_other.m_left_child;

            if (m_right_child != a_other.m_right_child)
                return m_right_child < a_other.m_right_child;

            return false;
            
        }
        
    };

    class global_node_sink
    {
        static std::set<node>* s_nodes;

    public:
        static const node* commit(
            node&& a_node
        )
        {
            /// This insertion will contract
            ///     any identical expressions.
            return &*s_nodes->insert(
                a_node
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

    inline const node* literal(
        uint32_t a_variable_index,
        bool a_sign
    )
    {
        if (a_variable_index == 0)
        {
            /// Base case for recursion. Just construct
            ///     the node with an edge in the signed direction.
            return global_node_sink::commit(
                node
                {
                    .m_left_child = !a_sign ? ONE : ZERO,
                    .m_right_child = a_sign ? ONE : ZERO
                }
            );
        }

        /// Make the recursive call. The resulting 
        ///     node will have BOTH children point
        ///     to the single result child.
        const node* l_result_child =
            literal(a_variable_index - 1, a_sign);

        /// Construct the result node and return it.
        return global_node_sink::commit(
            node
            {
                .m_left_child = l_result_child,
                .m_right_child = l_result_child
            }
        );
        
    }

    inline const node* invert(
        const node* a_node,
        std::map<const node*, const node*>& a_cache
    )
    {
        /// Invert the simple cases.
        if (a_node == ZERO)
            return ONE;

        if (a_node == ONE)
            return ZERO;

        /// Query the cache and if it is not
        ///     found, store the computed result.
        return CACHE(
            a_cache,
            a_node,
            global_node_sink::commit(
                node
                {
                    .m_left_child = invert(a_node->m_left_child, a_cache),
                    .m_right_child = invert(a_node->m_right_child, a_cache)
                }
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
        /// Check simple cases.
        if (a_x == ZERO)
            return a_y;

        if (a_y == ZERO)
            return a_x;

        if (a_x == ONE || a_y == ONE)
            return ONE;

        // /// Perform a lookup in the cache
        // ///    and if not found, do recursion.
        // const node* l_result = CACHE(
        //     a_cache,
        //     std::set({a_x, a_y}),
        //     global_node_sink::commit(
        //         node
        //         {
        //             .m_left_child = disjoin(a_x->m_left_child, a_y->m_left_child, a_cache),
        //             .m_right_child = disjoin(a_x->m_right_child, a_y->m_right_child, a_cache)
        //         }
        //     )
        // );

        const node* l_result_left =
            CACHE(
                a_cache,
                std::set({a_x->m_left_child, a_y->m_left_child}),
                disjoin(a_x->m_left_child, a_y->m_left_child, a_cache)
            );
        
        const node* l_result_right =
            CACHE(
                a_cache,
                std::set({a_x->m_right_child, a_y->m_right_child}),
                disjoin(a_x->m_right_child, a_y->m_right_child, a_cache)
            );

        /// Last-minute simplification. If BOTH children
        ///     of the disjunction are 1, return 1.
        ///     this simplification must require that both
        ///     children are antident, since if only one
        ///     was antident, we would care to preserve the
        ///     full details of the tree.
        if (l_result_left == ONE && l_result_right == ONE)
            return ONE;

        return l_result;

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

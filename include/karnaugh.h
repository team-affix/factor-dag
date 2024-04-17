#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ostream>

namespace karnaugh
{

    ////////////////////////////////////////////
    /////////////// INPUT TYPES ////////////////
    ////////////////////////////////////////////
    #pragma region INPUT TYPES

    typedef std::vector<bool> input;

    // inline coverage make_coverage(
    //     const std::set<input>& a_zeroes,
    //     const std::set<input>& a_ones
    // )
    // {
    //     coverage l_result;
        
    //     for (const auto& a_zero : a_zeroes)
    //         l_result.m_zeroes.insert((const input*)&a_zero);
            
    //     for (const auto& a_one : a_ones)
    //         l_result.m_ones.insert((const input*)&a_one);

    //     return l_result;
                
    // }

    typedef uint32_t literal;

    inline uint32_t index(
        literal a_literal
    )
    {
        return a_literal >> 1;
    }
    
    inline bool sign(
        literal a_literal
    )
    {
        return a_literal % 2;
    }

    inline bool covers(
        literal a_literal,
        const input& a_input
    )
    {
        return a_input.at(index(a_literal)) == sign(a_literal);
    }

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

    class tree
    {
        std::map<std::pair<size_t, literal>, tree> m_realized_subtrees;

        bool m_satisfiable;

    public:
        tree(
            const std::set<literal>& a_remaining_literals,
            const std::set<const input*>& a_zero_cover,
            const std::set<const input*>& a_one_block
        ) :
            m_satisfiable(a_one_block.size() > 0)
        {
            /// Base case of recursion.
            if (a_zero_cover.size() == 0 ||
                a_one_block.size() == 0)
                return;
            
            /////////////////////////////////////////////////////
            /// 1. Determine literal coverages given our coverage.
            /////////////////////////////////////////////////////
            
            #pragma region DETERMINE SUBCOVERAGES

            /////////////////////////////////////////////////////
            /// 1. Populate subcoverage map based on the
            ///     dissatisfying coverage of each literal.
            /////////////////////////////////////////////////////

            /// Key type is std::pair<size_t, literal> because we
            ///     can populate size_t with dissatisfying cov size.
            ///     This will ensure the map is sorted by minimum
            ///     dissatisfying coverage.

            /// Construct the structure containing covers of zeroes.
            std::map<std::pair<size_t, literal>, std::set<const input*>>
                l_zero_subcovers;
            
            for (literal l_literal : a_remaining_literals)
            {
                std::set<const input*> l_zero_subcover;
                
                /// Filter the dissatisfying coverage
                ///     based on the literal.
                std::copy_if(
                    a_zero_cover.begin(),
                    a_zero_cover.end(),
                    std::inserter(
                        l_zero_subcover,
                        l_zero_subcover.begin()
                    ),
                    [l_literal](
                        const input* a_zero
                    )
                    {
                        return covers(l_literal, *a_zero);
                    }
                );

                l_zero_subcovers[
                    std::pair {
                        l_zero_subcover.size(),
                        l_literal
                    }
                ] = l_zero_subcover;

            }

            //////////////////////////////////////////////////////
            /// 2. Determine where the satisfying inputs will go.
            //////////////////////////////////////////////////////

            /// Construct the structure containing blocks of ones.
            std::map<std::pair<size_t, literal>, std::set<const input*>>
                l_one_subblocks;

            for (const input* l_one : a_one_block)
            {
                auto l_insertion_position =
                    std::find_if(
                        l_zero_subcovers.begin(),
                        l_zero_subcovers.end(),
                        [l_one](
                            const auto& a_entry
                        )
                        {
                            return covers(a_entry.first.second, *l_one);
                        }
                    );

                l_one_subblocks[l_insertion_position->first].insert(l_one);
                
            }

            #pragma endregion

            /////////////////////////////////////////////////////
            /// 2. Realize ALL subtrees from the map
            /////////////////////////////////////////////////////

            #pragma region REALIZE SUBTREES

            for (const auto& [l_pair, l_cover] : l_zero_subcovers)
            {
                std::set<literal> l_subtree_remaining_literals;

                /// Filter all remaining literals based on
                ///     literal that is being taken care of
                ///     by this edge to the subtree.
                std::copy_if(
                    a_remaining_literals.begin(),
                    a_remaining_literals.end(),
                    std::inserter(
                        l_subtree_remaining_literals,
                        l_subtree_remaining_literals.begin()),
                    [l_pair](
                        literal a_literal
                    )
                    {
                        return index(a_literal) != index(l_pair.second);
                    }
                );

                m_realized_subtrees.emplace(
                    l_pair,
                    tree(
                        l_subtree_remaining_literals,
                        l_cover,
                        l_one_subblocks[l_pair]
                    )
                );

            }

            #pragma endregion

        }

        bool operator()(
            const input& a_input
        ) const
        {
            /// Base case of recursion.
            ///     We reached a leaf node
            ///     that has been fully realized.
            if (m_realized_subtrees.size() == 0)
                return m_satisfiable;

            /////////////////////////////////////////////////////
            /////// RECURSIVE CALL TO ALL COVERING PATHS ////////
            /////////////////////////////////////////////////////

            return std::any_of(
                m_realized_subtrees.begin(),
                m_realized_subtrees.end(),
                [&a_input](
                    const auto& a_entry
                )
                {
                    const literal& l_literal = a_entry.first.second;
                    const tree& l_tree = a_entry.second;
                    return covers(l_literal, a_input) && l_tree(a_input);
                }
            );

        }

        friend std::ostream& operator<<(
            std::ostream& a_ostream,
            const tree& a_tree
        )
        {
            if (a_tree.m_realized_subtrees.size() == 0)
                return a_ostream;

            bool l_add_separator = false;

            for (const auto& [l_pair, l_subtree] : a_tree.m_realized_subtrees)
            {
                if (!l_subtree.m_satisfiable)
                    continue;

                if (l_add_separator)
                    a_ostream << "+";

                if (l_subtree.m_realized_subtrees.size() > 0)
                    /// NOT a leaf node
                    a_ostream << l_pair.second << "(" << l_subtree << ")";
                else
                    /// IS a leaf node
                    a_ostream << l_pair.second;

                l_add_separator = true;
                
            }

            return a_ostream;

        }

    };

    inline tree generalize(
        const size_t a_variable_count,
        const std::set<input>& a_zeroes,
        const std::set<input>& a_ones
    )
    {
        /// Construct pointers to inputs.
        std::set<const input*> l_zero_pointers;
        std::set<const input*> l_one_pointers;
        
        for (const auto& a_zero : a_zeroes)
            l_zero_pointers.insert((const input*)&a_zero);
            
        for (const auto& a_one : a_ones)
            l_one_pointers.insert((const input*)&a_one);

        /// Generate starting set of literals
        ///     for generalization.
        std::set<literal> l_literals;

        const int l_literal_count = 2 * a_variable_count;

        for (int i = 0; i < l_literal_count; i++)
            l_literals.insert(i);

        return tree(
            l_literals,
            l_zero_pointers,
            l_one_pointers
        );

    }

    #pragma endregion
    
}

#endif

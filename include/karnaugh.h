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
#include "karnaugh_utils.h"

namespace karnaugh
{

    ////////////////////////////////////////////
    /////////////// INPUT TYPES ////////////////
    ////////////////////////////////////////////
    #pragma region INPUT TYPES

    typedef std::vector<bool> input;

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

    inline std::set<literal> make_literals(
        const int a_variable_count
    )
    {
        std::set<literal> l_result;

        const int l_literal_count = 2 * a_variable_count;

        for (int i = 0; i < l_literal_count; i++)
            l_result.insert(i);

        return l_result;
        
    }

    #pragma endregion

    ////////////////////////////////////////////
    ///////////////// MODELING /////////////////
    ////////////////////////////////////////////
    #pragma region MODELING

    class model
    {
        std::map<std::pair<size_t, literal>, model> m_realized_subtrees;

        bool m_satisfiable;

        model(
            const std::set<literal>& a_remaining_literals,
            const std::set<const input*>& a_zeroes,
            const std::set<const input*>& a_ones
        ) :
            m_satisfiable(a_ones.size() > 0)
        {
            /// Base case of recursion.
            if (a_zeroes.size() == 0 ||
                a_ones.size() == 0)
                return;
            
            /////////////////////////////////////////////////////
            /// 1. Group each zero into subsets,
            ///      defined by coverage by a literal.
            /////////////////////////////////////////////////////
            
            std::map<literal, std::set<const input*>> l_zero_cover =
                utils::group_by<literal, const input*>(
                    a_zeroes,
                    [&a_remaining_literals](
                        const input* a_zero
                    )
                    {
                        std::set<literal> l_result;

                        for (const literal l_literal : a_remaining_literals)
                        {
                            if (covers(l_literal, *a_zero))
                                l_result.insert(l_literal);
                        }

                        return l_result;
                        
                    }
                );
            
            //////////////////////////////////////////////////////
            /// 2. Sort literals in ascending dissatisfying cov. size.
            //////////////////////////////////////////////////////
            
            /// Key type is std::pair<size_t, literal> because we
            ///     can populate size_t with dissatisfying cov size.
            ///     This will ensure the set is sorted by minimum
            ///     dissatisfying coverage.
            std::set<std::pair<size_t, literal>> l_sorted_literals;

            for (const literal l_literal : a_remaining_literals)
                l_sorted_literals.emplace(l_zero_cover[l_literal].size(), l_literal);

            //////////////////////////////////////////////////////
            /// 3. Partition the ones based on the literal
            ///     that has minimum dissatisfying coverage 
            ///     that simultaneously covers it.
            //////////////////////////////////////////////////////

            std::map<literal, std::set<const input*>> l_one_partition =
                utils::group_by<literal, const input*>(
                    a_ones,
                    [&l_sorted_literals](
                        const input* a_input
                    )
                    {
                        auto l_first_covering_literal =
                            std::find_if(
                                l_sorted_literals.begin(),
                                l_sorted_literals.end(),
                                [a_input](
                                    const auto& a_entry
                                )
                                {
                                    return covers(a_entry.second, *a_input);
                                }
                            );

                        return l_first_covering_literal->second;
                        
                    }
                );

            /////////////////////////////////////////////////////
            /// 4. Realize ALL subtrees.
            /////////////////////////////////////////////////////

            #pragma region REALIZE SUBTREES

            for (const auto& [l_coverage_size, l_literal] : l_sorted_literals)
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
                    [l_literal](
                        literal a_literal
                    )
                    {
                        return index(a_literal) != index(l_literal);
                    }
                );

                m_realized_subtrees.emplace(
                    std::pair(l_coverage_size, l_literal),
                    model(
                        l_subtree_remaining_literals,
                        l_zero_cover[l_literal],
                        l_one_partition[l_literal]
                    )
                );

            }

            #pragma endregion

        }

    public:
        model(
            const size_t a_variable_count,
            const std::set<input>& a_zeroes,
            const std::set<input>& a_ones
        ) :
            model(
                make_literals(a_variable_count),
                utils::pointers(a_zeroes),
                utils::pointers(a_ones)
            )
        {

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
                    const model& l_tree = a_entry.second;
                    return covers(l_literal, a_input) && l_tree(a_input);
                }
            );

        }

        friend std::ostream& operator<<(
            std::ostream& a_ostream,
            const model& a_tree
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

    #pragma endregion
    
}

#endif

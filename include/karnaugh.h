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
            /// 1. Determine literal coverages given our coverage.
            /////////////////////////////////////////////////////
            
            #pragma region DETERMINE SUBCOVERAGES

            struct coverage
            {
                std::set<const input*> m_zeroes;
                std::set<const input*> m_ones;
            };
            
            /// Key type is std::pair<size_t, literal> because we
            ///     can populate size_t with dissatisfying cov size.
            ///     This will ensure the map is sorted by minimum
            ///     dissatisfying coverage.
            std::map<std::pair<size_t, literal>, coverage> l_subcoverages;

            /////////////////////////////////////////////////////
            /// 1. Populate subcoverage map based on the
            ///     dissatisfying coverage of each literal.
            /////////////////////////////////////////////////////
            
            for (literal l_literal : a_remaining_literals)
            {
                coverage l_literal_coverage;
                
                /// Filter the dissatisfying coverage
                ///     based on the literal.
                std::copy_if(
                    a_zeroes.begin(),
                    a_zeroes.end(),
                    std::inserter(
                        l_literal_coverage.m_zeroes,
                        l_literal_coverage.m_zeroes.begin()
                    ),
                    [l_literal](
                        const input* a_zero
                    )
                    {
                        return covers(l_literal, *a_zero);
                    }
                );

                l_subcoverages.emplace(
                    std::pair{
                        l_literal_coverage.m_zeroes.size(),
                        l_literal
                    },
                    l_literal_coverage
                );
                
            }

            //////////////////////////////////////////////////////
            /// 2. Populate satisfying coverage in the map entries
            //////////////////////////////////////////////////////

            for (const input* l_one : a_ones)
            {
                auto l_insertion_position =
                    std::find_if(
                        l_subcoverages.begin(),
                        l_subcoverages.end(),
                        [l_one](
                            const auto& a_entry
                        )
                        {
                            return covers(a_entry.first.second, *l_one);
                        }
                    );

                l_insertion_position->second.m_ones.insert(l_one);
                
            }

            #pragma endregion

            /////////////////////////////////////////////////////
            /// 2. Realize ALL subtrees from the map
            /////////////////////////////////////////////////////

            #pragma region REALIZE SUBTREES

            for (const auto& [l_pair, l_coverage] : l_subcoverages)
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
                    model(
                        l_subtree_remaining_literals,
                        l_coverage.m_zeroes,
                        l_coverage.m_ones
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
                make_pointers(a_zeroes),
                make_pointers(a_ones)
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

    private:
        static std::set<literal> make_literals(
            const size_t a_variable_count
        )
        {
            std::set<literal> l_result;

            const int l_literal_count = 2 * a_variable_count;

            for (int i = 0; i < l_literal_count; i++)
                l_result.insert(i);

            return l_result;
            
        }

        static std::set<const input*> make_pointers(
            const std::set<input>& a_inputs
        )
        {
            std::set<const input*> l_result;

            for (const input& l_input : a_inputs)
                l_result.insert(&l_input);

            return l_result;
            
        }

    };

    #pragma endregion
    
}

#endif

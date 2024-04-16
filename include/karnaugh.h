#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace karnaugh
{

    ////////////////////////////////////////////
    /////////////// INPUT TYPES ////////////////
    ////////////////////////////////////////////
    #pragma region INPUT TYPES

    typedef std::vector<bool> input;

    typedef input zero;
    
    typedef input one;

    struct coverage
    {
        std::set<const zero*> m_zeroes;
        std::set<const one*> m_ones;
    };

    inline coverage make_coverage(
        const std::set<zero>& a_zeroes,
        const std::set<one>& a_ones
    )
    {
        coverage l_result;
        
        for (const auto& a_zero : a_zeroes)
            l_result.m_zeroes.insert((const zero*)&a_zero);
            
        for (const auto& a_one : a_ones)
            l_result.m_ones.insert((const one*)&a_one);

        return l_result;
                
    }

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
        const input* a_input
    )
    {
        return a_input->at(index(a_literal)) == sign(a_literal);
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

    class tree
    {
        std::map<std::pair<size_t, literal>, tree> m_realized_subtrees;

        bool m_noncontradictory;

    public:
        tree(
            const std::set<literal>& a_remaining_literals,
            const coverage& a_coverage
        ) :
            m_noncontradictory(a_coverage.m_ones.size() > 0)
        {
            /// Base case of recursion.
            if (a_coverage.m_zeroes.size() == 0 ||
                a_coverage.m_ones.size() == 0)
                return;
            
            /////////////////////////////////////////////////////
            /// 1. Determine literal coverages given our coverage.
            /////////////////////////////////////////////////////
            
            std::map<std::pair<size_t, literal>, coverage> l_subcoverages =
                subcoverages(
                    a_remaining_literals,
                    a_coverage
                );

            /////////////////////////////////////////////////////
            /// 2. Realize ONLY the subtrees along trajectories.
            ///     NOTE: All literals which are not on trajectories
            ///     are already absent from the map's keys.
            /////////////////////////////////////////////////////

            realize_subtrees(
                a_remaining_literals,
                l_subcoverages
            );

        }

        bool evaluate(
            const input* a_input
        ) const
        {
            /// Base case of recursion.
            ///     We reached a leaf node
            ///     that has been fully realized.
            if (m_realized_subtrees.size() == 0)
                return m_noncontradictory;

            /////////////////////////////////////////////////////
            /// 1. Determine trajectory of input.
            /////////////////////////////////////////////////////

            std::map<std::pair<size_t, literal>, tree>::const_iterator
                l_submodel = std::find_if(
                    m_realized_subtrees.begin(),
                    m_realized_subtrees.end(),
                    [a_input](
                        const auto& l_entry
                    )
                    {
                        return covers(l_entry.first.second, a_input);
                    }
                );

            /// If the next node has not been realized,
            ///     then we return identity of disjunction (false).
            if (l_submodel == m_realized_subtrees.end())
                return false;

            /////////////////////////////////////////////////////
            /// 2. Send the input down its trajectory.
            /////////////////////////////////////////////////////

            return l_submodel->second.evaluate(a_input);
            
        }

        bool evaluate(
            const input& a_input
        ) const
        {
            return evaluate(&a_input);
        }

    private:
        static std::map<std::pair<size_t, literal>, coverage> subcoverages(
            const std::set<literal>& a_literals,
            const coverage& a_coverage
        )
        {
            /// Key type is std::pair<size_t, literal> because we
            ///     can populate size_t with dissatisfying cov size.
            ///     This will ensure the map is sorted by minimum
            ///     dissatisfying coverage.
            std::map<std::pair<size_t, literal>, coverage> l_result;

            /////////////////////////////////////////////////////
            /// 1. Populate subcoverage map based on the
            ///     dissatisfying coverage of each literal.
            /////////////////////////////////////////////////////
            
            for (literal l_literal : a_literals)
            {
                coverage l_literal_coverage;
                
                /// Filter the dissatisfying coverage
                ///     based on the literal.
                std::copy_if(
                    a_coverage.m_zeroes.begin(),
                    a_coverage.m_zeroes.end(),
                    std::inserter(
                        l_literal_coverage.m_zeroes,
                        l_literal_coverage.m_zeroes.begin()
                    ),
                    [l_literal](
                        const zero* a_zero
                    )
                    {
                        return covers(l_literal, a_zero);
                    }
                );

                l_result.emplace(
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

            for (const one* l_one : a_coverage.m_ones)
            {
                auto l_insertion_position =
                    std::find_if(
                        l_result.begin(),
                        l_result.end(),
                        [l_one](
                            const auto& a_entry
                        )
                        {
                            return covers(a_entry.first.second, l_one);
                        }
                    );

                l_insertion_position->second.m_ones.insert(l_one);
                
            }

            return l_result;
            
        }

        void realize_subtrees(
            const std::set<literal>& a_remaining_literals,
            const std::map<std::pair<size_t, literal>, coverage>& a_subcoverages
        )
        {
            for (const auto& [l_pair, l_coverage] : a_subcoverages)
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
                        l_coverage
                    )
                );

            }

        }
        
    };
    
    #pragma endregion
    
}

#endif

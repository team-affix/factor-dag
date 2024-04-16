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

    typedef input dissatisfying_input;
    
    typedef input satisfying_input;

    #pragma endregion

    ////////////////////////////////////////////
    ///////////////// MODELING /////////////////
    ////////////////////////////////////////////
    #pragma region MODELING

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

    struct coverage
    {
        std::set<const dissatisfying_input*> m_zeroes;
        std::set<const satisfying_input*> m_ones;
    };

    class model
    {
        std::map<std::pair<size_t, literal>, model> m_realized_subtrees;

    public:
        model(
            const std::set<literal>& a_remaining_literals,
            const coverage& a_coverage
        )
        {
            /// Base case of recursion.
            if (a_coverage.m_zeroes.size() == 0)
                return;
            
            /// 1. Determine literal coverages of unsatisfying inputs.
            
            std::map<std::pair<size_t, literal>, coverage> l_subcoverages =
                subcoverages(
                    a_remaining_literals,
                    a_coverage
                );

            /// 2. Realize ONLY the subtrees along trajectories.
            ///     NOTE: All literals which are not on trajectories
            ///     are already absent from the map's keys.

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
                return true;

            /// 1. Determine trajectory of input.
            std::map<std::pair<size_t, literal>, model>::const_iterator
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

            /// 2. Send the input down its trajectory.
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
            std::map<std::pair<size_t, literal>, coverage> l_result;

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
                        const dissatisfying_input* a_zero
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

            for (const satisfying_input* l_one : a_coverage.m_ones)
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

            std::map<std::pair<size_t, literal>, coverage>::iterator
                l_removal_iterator = l_result.begin();

            while (l_removal_iterator != l_result.end())
                l_removal_iterator =
                    l_removal_iterator->second.m_ones.size() == 0 ?
                    l_result.erase(l_removal_iterator) :
                    ++l_removal_iterator;

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
                    model(
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

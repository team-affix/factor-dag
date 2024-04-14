#ifndef LOGIC_H
#define LOGIC_H

#include <set>
#include <vector>
#include <algorithm>
#include "input.h"

namespace karnaugh
{

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

    /// Construction of root: dct(remaining_literals, remaining_dis_cov)

    class dissatisfying_coverage_tree
    {
        std::map<literal, dissatisfying_coverage_tree> m_realized_subtrees;

    public:
        dissatisfying_coverage_tree(
            const std::set<literal>& a_remaining_literals,
            const std::set<const dissatisfying_input*>& a_remaining_coverage,
            const std::set<const satisfying_input*>& a_satisfying_inputs
        )
        {
            /// Base case of recursion.
            if (a_remaining_coverage.size() == 0)
                return;
            
            /// 1. determine literal coverage of unsatisfying inputs.
            
            std::map<literal, std::set<const dissatisfying_input*>> l_subcoverages =
                subcoverages(
                    a_remaining_literals,
                    a_remaining_coverage
                );
            
            /// 2. determine the satisfying input trajectories.

            std::map<literal, std::set<const satisfying_input*>> l_trajectories =
                trajectories(
                    order_coverages(l_subcoverages),
                    a_satisfying_inputs
                );
            
            /// 3. realize only the subtrees along trajectories.

            for (const auto& [l_remaining_literal, l_satisfying_inputs] : l_trajectories)
            {
                std::set<literal> l_subtree_remaining_literals;

                /// Filter all remaining literals based on
                ///     literal that is being taken care of
                ///     by this edge to the subtree.
                std::copy_if(
                    a_remaining_literals.begin(),
                    a_remaining_literals.end(),
                    std::inserter(l_subtree_remaining_literals, l_subtree_remaining_literals.begin()),
                    [l_remaining_literal](
                        literal a_literal
                    )
                    {
                        return index(a_literal) != index(l_remaining_literal);
                    }
                );
                
                /// Realize the subtree.
                m_realized_subtrees.emplace(
                    l_remaining_literal, 
                    dissatisfying_coverage_tree(
                        l_subtree_remaining_literals,
                        l_subcoverages[l_remaining_literal],
                        l_trajectories[l_remaining_literal]
                    )
                );
            }

        }

        // bool evaluate(
        //     const input& a_input
        // )
        // {
        //     /// Base case of recursion.
        //     ///     We reached a leaf node
        //     ///     that has been fully realized.
        //     if (m_realized_subtrees.size() == 0)
        //         return true;

            
            
        // }

    private:
        static std::map<literal, std::set<const dissatisfying_input*>> subcoverages(
            const std::set<literal>& a_literals,
            const std::set<const dissatisfying_input*>& a_dissatisfying_coverage
        )
        {
            std::map<literal, std::set<const dissatisfying_input*>> l_result;

            for (literal l_remaining_literal : a_literals)
            {
                /// Filter the dissatisfying coverage
                ///     based on the literal.
                std::copy_if(
                    a_dissatisfying_coverage.begin(),
                    a_dissatisfying_coverage.end(),
                    std::inserter(
                        l_result[l_remaining_literal],
                        l_result[l_remaining_literal].begin()
                    ),
                    [l_remaining_literal](
                        const dissatisfying_input* a_dissatisfying_input
                    )
                    {
                        return covers(l_remaining_literal, a_dissatisfying_input);
                    }
                );
            }

            return l_result;
            
        }

        static std::vector<literal> order_coverages(
            const std::map<literal, std::set<const dissatisfying_input*>>& a_subcoverages
        )
        {
            std::vector<literal> l_result;

            /// Push all the literals from the map
            ///     into the vector.
            for (const auto& [l_literal, _] : a_subcoverages)
                l_result.push_back(l_literal);


            /// Stable sort should be used, since
            ///     there will be multiple literals
            ///     with equivalent coverage sizes.
            ///     (Stable sort provides non-descending order)
            std::stable_sort(
                l_result.begin(),
                l_result.end(),
                [&a_subcoverages](
                    literal a_literal_0,
                    literal a_literal_1
                )
                {
                    return
                        a_subcoverages.at(a_literal_0).size() <
                        a_subcoverages.at(a_literal_1).size();
                }
            );

            return l_result;
            
        }

        static std::map<literal, std::set<const satisfying_input*>> trajectories(
            const std::vector<literal>& a_ordered_literals,
            const std::set<const satisfying_input*>& a_satisfying_inputs
        )
        {
            /// NOTE: Unused literals will be absent
            ///       from the set of keys in the result.
            std::map<literal, std::set<const satisfying_input*>> l_result;

            for (const satisfying_input* l_satisfying_input : a_satisfying_inputs)
            {
                /// Since the vector is already sorted based
                ///     on minimum dissatisfying coverage,
                ///     we can just accept the first occurance
                ///     of a covering literal in a forward scan.
                std::vector<literal>::const_iterator
                    l_trajectory = std::find_if(
                        a_ordered_literals.begin(),
                        a_ordered_literals.end(),
                        [l_satisfying_input](
                            literal a_literal
                        )
                        {
                            return covers(a_literal, l_satisfying_input);
                        }
                    );

                /// Insert the satisfying input into the set,
                ///     specifying the trajectory.
                l_result[*l_trajectory].insert(l_satisfying_input);

            }

            return l_result;
            
        }
        
    };
    
}

#endif

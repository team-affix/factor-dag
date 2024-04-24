#ifndef DAG_H
#define DAG_H

#include <stdint.h>
#include <ostream>

#define ONE (const dag::node*)-1
#define ZERO (const dag::node*)nullptr

namespace dag
{
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

    inline std::ostream& operator<<(
        std::ostream& a_ostream,
        const node* a_node
    )
    {
        /// Do not print base cases.
        if (a_node == ZERO || a_node == ONE)
            return a_ostream;

        /// Only print bounding parens if BOTH children
        ///     are non-zero quantities.
        if (a_node->negative() != ZERO && a_node->positive() != ZERO)
            a_ostream << "(";

        /// Negative case. Print an apostrophe to indicate.
        if (a_node->negative() != ZERO)
            a_ostream << a_node->depth() << "'" << a_node->negative();

        /// Only print disjunction if BOTH children
        ///     are non-zero quantities.
        if (a_node->negative() != ZERO && a_node->positive() != ZERO)
            a_ostream << "+";

        /// Positive case. Omit apostrophe to indicate.
        if (a_node->positive() != ZERO)
            a_ostream << a_node->depth() << a_node->positive();

        /// Closing paren.
        if (a_node->negative() != ZERO && a_node->positive() != ZERO)
            a_ostream << ")";
        
        return a_ostream;
        
    }

}

#endif

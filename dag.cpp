#include <assert.h>

#include "include/dag.h"

namespace dag
{
    std::ostream& operator<<(
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
            a_ostream << "[" << a_node->depth() << "]'" << a_node->negative();

        /// Only print disjunction if BOTH children
        ///     are non-zero quantities.
        if (a_node->negative() != ZERO && a_node->positive() != ZERO)
            a_ostream << "+";

        /// Positive case. Omit apostrophe to indicate.
        if (a_node->positive() != ZERO)
            a_ostream << "[" << a_node->depth() << "]" << a_node->positive();

        /// Closing paren.
        if (a_node->negative() != ZERO && a_node->positive() != ZERO)
            a_ostream << ")";
        
        return a_ostream;
        
    }

    std::istream& operator>>(
        std::istream& a_istream,
        const node*& a_node
    )
    {
        char l_current_char = '\0';

        /// Construct the result node.
        a_node = ONE;

        while(a_istream.get(l_current_char))
        {
            const node* l_subexpression;
            
            switch (l_current_char)
            {
                case '\0': { return a_istream; }
                case ')' : { return a_istream; }
                case '(' :
                {
                    /// This will pop the entire subexpression,
                    ///     including the associated closing paren.
                    a_istream >> l_subexpression;

                    break;
                
                }
                case '[':
                {
                    uint32_t l_variable_index = 0;
                    
                    a_istream >> l_variable_index;
                    
                    l_subexpression = literal(l_variable_index, true);

                    /// Should remove the closing bracket ']'.
                    assert(a_istream.get() == ']');

                    break;

                }
                case '+':
                {
                    /// Should pop entire expression of this
                    ///     level, starting from + until end.
                    a_istream >> l_subexpression;
                    
                    a_node = logic::disjoin(a_node, l_subexpression);
                    
                    /// Since this entire level is parsed,
                    ///     we must return.
                    return a_istream;

                }
                default:
                {
                    /// No subexpression has been extracted,
                    ///     so we mustn't reach the post-switch code.
                    continue;
                }
            }

            /// Check for trailing apostrophe (inversion).
            if (a_istream.peek() == '\'')
            {
                /// Pops the quote.
                a_istream.get();

                /// Invert the subexpression.
                l_subexpression = logic::invert(l_subexpression);

            }

            /// Finally, conjoin the subexpression to the result.
            a_node = logic::conjoin(a_node, l_subexpression);
            
        }
        
        return a_istream;
        
    }

    std::set<dag::node>* global_node_sink::s_nodes(nullptr);

}

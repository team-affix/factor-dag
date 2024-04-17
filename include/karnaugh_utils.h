#ifndef KARNAUGH_UTILS_H
#define KARNAUGH_UTILS_H

#include <set>
#include <map>
#include <functional>

namespace utils
{
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

    // template<typename T>
    // inline std::set<T> filter(
    //     const std::set<T>& a_vals,
    //     const std::function<bool(T)>& a_query
    // )
    // {
    //     std::set<T> l_result;

    //     std::
        
    // }

    template<typename KEY, typename VALUE>
    inline std::map<KEY, std::set<VALUE>> group_by(
        const std::set<VALUE>& a_values,
        const std::function<std::set<KEY>(VALUE)>& a_grouper
    )
    {
        std::map<KEY, std::set<VALUE>> l_result;

        for (const VALUE& l_value : a_values)
        {
            std::set<KEY> l_keys = a_grouper(l_value);

            for (const KEY& l_key : l_keys)
                l_result[l_key].insert(l_value);
            
        }

        return l_result;
        
    }

    template<typename KEY, typename VALUE>
    inline std::map<KEY, std::set<VALUE>> group_by(
        const std::set<VALUE>& a_values,
        const std::function<KEY(VALUE)>& a_partitioner
    )
    {
        return group_by<KEY, VALUE>(
            a_values,
            [&a_partitioner](
                VALUE a_value
            )
            {
                return std::set<KEY> { a_partitioner(a_value) };
            }
        );
    }

};

#endif

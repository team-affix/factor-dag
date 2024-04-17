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

    template<typename VALUE, typename CALLABLE>
    inline auto cover(
        const std::set<VALUE>& a_values,
        const CALLABLE& a_grouper
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

    template<typename VALUE, typename CALLABLE>
    inline auto partition(
        const std::set<VALUE>& a_values,
        const CALLABLE& a_partitioner
    )
    {
        return cover(
            a_values,
            [&a_partitioner](
                VALUE a_value
            )
            {
                return std::set{ a_partitioner(a_value) };
            }
        );
    }

};

#endif

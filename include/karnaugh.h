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
#include <stack>
#include "dag.h"

#define CACHE(cache, key, value) \
    (cache.contains(key) ? cache[key] : cache[key] = value)

namespace logic
{

    ////////////////////////////////////////////
    //////////// UTILITY FUNCTIONS /////////////
    ////////////////////////////////////////////
    #pragma region UTILITY FUNCTIONS

    /// Constructs a set of pointers given
    ///     a set of any type.
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

    /// Filters the inputted set by
    ///     the supplied predicate.
    template<typename T, typename FUNCTION>
    inline std::set<T> filter(
        const std::set<T>& a_vals,
        const FUNCTION& a_query
    )
    {
        std::set<T> l_result;

        std::copy_if(
            a_vals.begin(),
            a_vals.end(),
            std::inserter(
                l_result,
                l_result.begin()
            ),
            a_query
        );

        return l_result;
        
    }

    /// Returns a cover (see set theory)
    ///     of an inputted set, grouped
    ///     by the supplied lambda.
    template<typename VALUE, typename FUNCTION>
    inline auto cover(
        const std::set<VALUE>& a_values,
        const FUNCTION& a_grouper
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

    /// Returns a partition (see set theory)
    ///     of the inputted set, grouped by
    ///     the supplied lambda.
    template<typename VALUE, typename FUNCTION>
    inline auto partition(
        const std::set<VALUE>& a_values,
        const FUNCTION& a_partitioner
    )
    {
        return cover(
            a_values,
            [&a_partitioner](
                VALUE a_value
            )
            {
                return std::set({ a_partitioner(a_value) });
            }
        );
    }

    #pragma endregion

    ////////////////////////////////////////////
    /////////////// INPUT TYPES ////////////////
    ////////////////////////////////////////////
    #pragma region INPUT TYPES

    typedef std::vector<bool> input;

    #pragma endregion

    ////////////////////////////////////////////
    ///////////////// MODELING /////////////////
    ////////////////////////////////////////////
    #pragma region MODELING

    class global_node_sink
    {
        static std::set<node>* s_factors;

    public:
        static const node* emplace(
            uint32_t a_depth,
            const node* a_left_child,
            const node* a_right_child
        )
        {
            /// Apply simplification to factor.
            if (a_left_child == a_right_child)
                return a_left_child;

            /// This insertion will contract
            ///     any identical expressions.
            return &*s_factors->emplace(
                a_depth, a_left_child, a_right_child
            ).first;

        }

        static std::set<node>* bind(
            std::set<node>* a_factors
        )
        {
            /// Save the previously bound factor sink
            std::set<node>* l_result = s_factors;

            /// Bind to the new factor sink
            s_factors = a_factors;

            /// Return the factor sink that was unbound.
            return l_result;
            
        }
        
    };

    inline const node* literal(
        uint32_t a_variable_index,
        bool a_sign
    )
    {
        return
            global_node_sink::emplace(
                a_variable_index,
                !a_sign ? ONE : ZERO,
                a_sign ? ONE : ZERO
            );
    }

    inline const node* invert(
        std::map<const node*, const node*>& a_cache,
        const node* a_node
    )
    {
        if (a_node == ZERO)
            return ONE;
        if (a_node == ONE)
            return ZERO;

        /// Query the cache and if it is not
        ///     found, store the computed result.
        return CACHE(
            a_cache,
            a_node,
            global_node_sink::emplace(
                a_node->depth(),
                invert(a_cache, a_node->negative()),
                invert(a_cache, a_node->positive())
            )
        );

    }

    inline const node* invert(
        const node* a_node
    )
    {
        /// Construct the function cache.
        std::map<const node*, const node*> l_cache;

        /// Call the overload, supplying the cache.
        return invert(l_cache, a_node);
        
    }

    inline const node* join(
        std::map<std::set<const node*>, const node*>& a_cache,
        const node* a_ident,
        const node* a_antident,
        const node* a_x,
        const node* a_y
    )
    {
        /// If either operand is a zero,
        ///     return the opposite operand.
        if (a_x == a_ident)
            return a_y;
        if (a_y == a_ident)
            return a_x;

        /// If either operand is 1, just
        ///     return 1.
        if (a_x == a_antident || a_y == a_antident)
            return a_antident;

        /// We need to make variable the
        ///     nodes that we will recur on,
        ///     due to the potential for
        ///     differing node depths.
        const node* l_x_left = a_x->negative();
        const node* l_y_left = a_y->negative();
        const node* l_x_right = a_x->positive();
        const node* l_y_right = a_y->positive();

        /// If the depths differ, we mustn't
        ///     traverse to the children of
        ///     the higher-depth node.
        if (a_x->depth() > a_y->depth())
        {
            l_x_left = a_x;
            l_x_right = a_x;
        }
        else if (a_y->depth() > a_x->depth())
        {
            l_y_left = a_y;
            l_y_right = a_y;
        }

        /// Construct the cache key, which
        ///     should be the sorted pair:
        std::set<const node*> l_key = { a_x, a_y };

        return CACHE(
            a_cache,
            l_key,
            global_node_sink::emplace(
                std::min(a_x->depth(), a_y->depth()),
                join(a_cache, a_ident, a_antident, l_x_left, l_y_left),
                join(a_cache, a_ident, a_antident, l_x_right, l_y_right)
            )
        );

    }

    inline const node* join(
        const node* a_ident,
        const node* a_antident,
        const node* a_x,
        const node* a_y,
        auto ... a_remaining_operands
    )
    {
        /// Construct the function cache.
        std::map<std::set<const node*>, const node*> l_cache;

        /// Call the overload, supplying the cache.
        const node* l_junction = join(l_cache, a_ident, a_antident, a_x, a_y);

        if constexpr (sizeof...(a_remaining_operands) > 0)
            return join(a_ident, a_antident, l_junction, a_remaining_operands...);
        else
            return l_junction;
        
    }

    inline const node* disjoin(
        const node* a_x,
        const node* a_y,
        auto ... a_remaining_operands
    )
    {
        /// Call the overload, supplying the cache.
        return join(ZERO, ONE, a_x, a_y, a_remaining_operands...);
    }

    inline const node* conjoin(
        const node* a_x,
        const node* a_y,
        auto ... a_remaining_operands
    )
    {
        /// Call the overload, supplying the cache.
        return join(ONE, ZERO, a_x, a_y, a_remaining_operands...);
    }
    
    #pragma endregion
    
}

#endif

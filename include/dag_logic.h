#ifndef DAG_LOGIC_H
#define DAG_LOGIC_H

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
#include "../digital-logic/include/logic.h"

/// This macro function defines
///     getting a value from cache if key contained,
///     otherwise, computing value and caching it.
#define CACHE(cache, key, value) \
    (cache.contains(key) ? cache[key] : cache[key] = value)

namespace dag
{

    ////////////////////////////////////////////
    ////// FACTOR-DAG SPECIFIC FUNCTIONS ///////
    ////////////////////////////////////////////
    #pragma region USER-SPECIALIZED DAG LOGIC

    class global_node_sink
    {
        static std::set<dag::node>* s_factors;

    public:
        static const dag::node* emplace(
            uint32_t a_depth,
            const dag::node* a_left_child,
            const dag::node* a_right_child
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

        static std::set<dag::node>* bind(
            std::set<dag::node>* a_factors
        )
        {
            /// Save the previously bound factor sink
            std::set<dag::node>* l_result = s_factors;

            /// Bind to the new factor sink
            s_factors = a_factors;

            /// Return the factor sink that was unbound.
            return l_result;
            
        }
        
    };

    inline const dag::node* literal(
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

    inline const dag::node* join(
        std::map<std::set<const dag::node*>, const dag::node*>& a_cache,
        const dag::node* a_ident,
        const dag::node* a_antident,
        const dag::node* a_x,
        const dag::node* a_y
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
        const dag::node* l_x_left = a_x->negative();
        const dag::node* l_y_left = a_y->negative();
        const dag::node* l_x_right = a_x->positive();
        const dag::node* l_y_right = a_y->positive();

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
        std::set<const dag::node*> l_key = { a_x, a_y };

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

    inline const dag::node* invert(
        std::map<const dag::node*, const dag::node*>& a_cache,
        const dag::node* a_node
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

    #pragma endregion

}

namespace logic
{

    ////////////////////////////////////////////
    //////// USER-SPECIALIZED DAG LOGIC ////////
    ////////////////////////////////////////////
    #pragma region USER-SPECIALIZED DAG LOGIC

    template<>
    inline const dag::node* padding(
        bool a_logic_state
    )
    {
        return a_logic_state ? ONE : ZERO;
    }

    template<>
    inline const dag::node* join(
        bool a_identity,
        const dag::node* a_x,
        const dag::node* a_y
    )
    {
        /// Construct the function cache.
        std::map<std::set<const dag::node*>, const dag::node*> l_cache;

        return dag::join(
            l_cache,
            a_identity ? ONE : ZERO,
            a_identity ? ZERO : ONE,
            a_x,
            a_y
        );

    }

    template<>
    inline const dag::node* invert(
        const dag::node* a_node
    )
    {
        /// Construct the function cache.
        std::map<const dag::node*, const dag::node*> l_cache;

        /// Call the overload, supplying the cache.
        return dag::invert(l_cache, a_node);
        
    }

    #pragma endregion
    
}

#endif
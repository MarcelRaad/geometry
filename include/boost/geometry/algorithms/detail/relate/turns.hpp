// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/type_traits/is_base_of.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate { namespace turns {

// TURN_INFO

// TODO: rename distance_info to enriched_info or something like that
//       and add bool is_first indicating if the turn was generated on the first Point of a Range
//       maybe also bool is_last or some enum { first, middle, last }
//       This info could be used in turns analysis for Linestrings (no need to calculate this twice).
//       get_turn_info_ll must check this anyway.

template<typename P>
struct distance_info
{
    typedef typename strategy::distance::services::return_type
        <
            typename strategy::distance::services::comparable_type
                <
                    typename strategy::distance::services::default_strategy
                        <
                            point_tag,
                            P
                        >::type
                >::type,
            P, P
        >::type distance_type;

    inline distance_info()
        : distance(distance_type())
    {}

    distance_type distance; // distance-measurement from segment.first to IP
};

template <typename P>
struct turn_operation_with_distance : public overlay::turn_operation
{
    distance_info<P> enriched;
};



// GET_TURNS

template <typename Geometry1,
          typename Geometry2,
          typename GetTurnPolicy
            = detail::get_turns::get_turn_info_type<Geometry1, Geometry2, overlay::calculate_distance_policy> >
struct get_turns
{
    typedef typename geometry::point_type<Geometry1>::type point1_type;

    typedef overlay::turn_info
        <
            point1_type,
            turn_operation_with_distance<point1_type>
        > turn_info;

    template <typename Turns>
    static inline void apply(Turns & turns, Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        detail::get_turns::no_interrupt_policy interrupt_policy;

        static const bool reverse1 = detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value;
        static const bool reverse2 = detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value;

        dispatch::get_turns
            <
                typename bg::tag<Geometry1>::type, typename bg::tag<Geometry2>::type,
                Geometry1, Geometry2, reverse1, reverse2,
                GetTurnPolicy
            >::apply(0, geometry1, 1, geometry2, bg::detail::no_rescale_policy(), turns, interrupt_policy);
    }
};

// TURNS SORTING AND SEARCHING

// sort turns by G1 - source_index == 0 by:
// seg_id -> distance -> operation
template <int N = 0, int U = 1, int I = 2, int B = 3, int C = 4, int O = 0, std::size_t OpId = 0>
struct less_seg_dist_op
{
    BOOST_STATIC_ASSERT(OpId < 2);

    template <typename Op> static inline
    int order_op(Op const& op)
    {
        switch(op.operation)
        {
        case detail::overlay::operation_none : return N;
        case detail::overlay::operation_union : return U;
        case detail::overlay::operation_intersection : return I;
        case detail::overlay::operation_blocked : return B;
        case detail::overlay::operation_continue : return C;
        case detail::overlay::operation_opposite : return O;
        }
        return -1;
    }

    template <typename Op> static inline
    bool use_operation(Op const& left, Op const& right)
    {
        return order_op(left) < order_op(right);
    }

    template <typename Op> static inline
    bool use_other_multi_ring_id(Op const& left, Op const& right)
    {
        //return left.other_id.ring_index < right.other_id.ring_index;
        
        if ( left.other_id.ring_index == -1 )
        {
            if ( right.other_id.ring_index == -1 )
                return use_operation(left, right); // sort by operation
            else
                return true; // right always greater
        }
        else // left.other_id.ring_index != -1
        {
            if ( right.other_id.ring_index == -1 )
                return false; // left always greater

            // here both ring_indexes are greater than -1
            // so first, sort also by multi_index
            return left.other_id.multi_index < right.other_id.multi_index || (
                       left.other_id.multi_index == right.other_id.multi_index && (
                       left.other_id.ring_index < right.other_id.ring_index || (
                           left.other_id.ring_index == right.other_id.ring_index &&
                           use_operation(left, right) )
                    )
                );
        }
    }

    template <typename Op> static inline
    bool use_distance(Op const& left, Op const& right)
    {
        return left.enriched.distance < right.enriched.distance || (
                    geometry::math::equals(left.enriched.distance, right.enriched.distance) &&
                    use_other_multi_ring_id(left, right)
                );
    }

    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        segment_identifier const& sl = left.operations[OpId].seg_id;
        segment_identifier const& sr = right.operations[OpId].seg_id;

        return sl < sr || ( sl == sr && use_distance(left.operations[OpId], right.operations[OpId]) );
    }
};

//template <typename Turn> inline
//bool is_valid_method(Turn const& turn)
//{
//    return turn.method != detail::overlay::method_none
//        && turn.method != detail::overlay::method_disjoint
//        && turn.method != detail::overlay::method_error;
//}
//
//template <typename Turn> inline
//bool is_valid_operation(Turn const& turn)
//{
//    BOOST_ASSERT(!turn.has(detail::overlay::operation_opposite));
//    return !turn.both(detail::overlay::operation_none);
//}
//
//template <typename Turn> inline
//bool is_valid_turn(Turn const& turn)
//{
//    return is_valid_method(turn) && is_valid_operation(turn);
//}
//
//template <bool IsCyclic, typename TurnIt, typename Cond> inline
//TurnIt find_next_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
//{
//    if ( first == last )
//        return last;
//
//    if ( current != last )
//    {
//        TurnIt it = current;
//        ++it;
//
//        for ( ; it != last ; ++it )
//            if ( cond(*it) )
//                return it;
//    }
//
//    if ( IsCyclic )
//    {
//        for ( TurnIt it = first ; it != current ; ++it )
//            if ( cond(*it) )
//                return it;
//    }
//
//    return last;
//}
//
//template <bool IsCyclic, typename TurnIt, typename Cond> inline
//TurnIt find_previous_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
//{
//    typedef std::reverse_iterator<TurnIt> Rit;
//    Rit rlast = Rit(first);
//    if ( current == last )
//        return last;
//    ++current;
//    Rit res = find_next_if<IsCyclic>(Rit(last), Rit(current), rlast, cond);
//    if ( res == rlast )
//        return last;
//    else
//        return --res.base();
//}
//
//template <typename TurnIt, typename Cond> inline
//TurnIt find_first_if(TurnIt first, TurnIt last, Cond cond)
//{
//    return std::find_if(first, last, cond);
//}
//
//template <typename TurnIt, typename Cond> inline
//TurnIt find_last_if(TurnIt first, TurnIt last, Cond cond)
//{
//    typedef std::reverse_iterator<TurnIt> Rit;
//    Rit rlast = Rit(first);
//    Rit res = std::find_if(Rit(last), rlast, cond);
//    if ( res == rlast )
//        return last;
//    else
//        return --res.base();
//}

}}} // namespace detail::relate::turns
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
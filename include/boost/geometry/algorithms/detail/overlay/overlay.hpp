// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP


#include <deque>
#include <map>
#include <vector>

#include <boost/range.hpp>
#include <boost/mpl/assert.hpp>


#include <boost/geometry/algorithms/detail/overlay/assemble.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/reverse_operations.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>


#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/iterators/range_type.hpp>


#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
#  include <boost/geometry/util/write_dsv.hpp>
#endif


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{



template
<
    typename Geometry1, typename Geometry2,
    typename OutputIterator, typename GeometryOut,
    int Direction, order_selector Order,
    typename Strategy
>
struct overlay
{
    static inline OutputIterator apply(
                Geometry1 const& geometry1, Geometry2 const& geometry2,
                OutputIterator out,
                Strategy const& )
    {
        if (geometry::num_points(geometry1) == 0 && geometry::num_points(geometry2) == 0)
        {
            return out;
        }

        typedef typename geometry::point_type<GeometryOut>::type point_type;
        typedef detail::overlay::traversal_turn_info<point_type> turn_info;
        typedef std::deque<turn_info> container_type;

        // "Use" rangetype for ringtype:
        // for polygon, it is the type of the exterior ring.
        // for ring, it is the ring itself. That is what is
        // for multi-polygon, it is also the type of the ring.
        typedef typename geometry::range_type<GeometryOut>::type ring_type;

        // If one input is empty, output the other one for a union.
        // For an intersection, the intersection is empty.
        if (geometry::num_points(geometry1) == 0
            || geometry::num_points(geometry2) == 0)
        {
            if (Direction == 1)
            {
                std::map<ring_identifier, int> map;
                std::vector<ring_type> rings;
                return assemble<GeometryOut>(rings, map,
                                geometry1, geometry2, Direction, false, false, out);
            }
            return out;
        }

        container_type turn_points;

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "get turns" << std::endl;
#endif
        detail::get_turns::no_interrupt_policy policy;
        boost::geometry::get_turns
            <
                detail::overlay::calculate_distance_policy
            >(geometry1, geometry2, turn_points, policy);

        if (Order == counterclockwise)
        {
            detail::overlay::reverse_operations(turn_points);
        }

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "enrich" << std::endl;
#endif
        typename Strategy::side_strategy_type side_strategy;
        geometry::enrich_intersection_points(turn_points, geometry1, geometry2,
                    side_strategy);

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "traverse" << std::endl;
#endif
        std::vector<ring_type> rings;
        geometry::traverse<Order>(geometry1, geometry2,
                Direction == -1
                    ? boost::geometry::detail::overlay::operation_intersection
                    : boost::geometry::detail::overlay::operation_union
                    ,
                turn_points, rings);

        std::map<ring_identifier, int> map;
        map_turns(map, turn_points);
        return assemble<GeometryOut>(rings, map,
                        geometry1, geometry2, Direction, false, false, out);
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP
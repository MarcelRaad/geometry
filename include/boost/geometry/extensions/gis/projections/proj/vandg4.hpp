#ifndef BOOST_GEOMETRY_PROJECTIONS_VANDG4_HPP
#define BOOST_GEOMETRY_PROJECTIONS_VANDG4_HPP

// Boost.Geometry - extensions-gis-projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Original copyright notice:
 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry { namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace vandg4{ 
            static const double TOL = 1e-10;
            static const double TWORPI = 0.63661977236758134308;


            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_vandg4_spheroid : public base_t_f<base_vandg4_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;


                inline base_vandg4_spheroid(const Parameters& par)
                    : base_t_f<base_vandg4_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    double x1, t, bt, ct, ft, bt2, ct2, dt, dt2;
                
                    if (fabs(lp_lat) < TOL) {
                        xy_x = lp_lon;
                        xy_y = 0.;
                    } else if (fabs(lp_lon) < TOL || fabs(fabs(lp_lat) - HALFPI) < TOL) {
                        xy_x = 0.;
                        xy_y = lp_lat;
                    } else {
                        bt = fabs(TWORPI * lp_lat);
                        bt2 = bt * bt;
                        ct = 0.5 * (bt * (8. - bt * (2. + bt2)) - 5.)
                            / (bt2 * (bt - 1.));
                        ct2 = ct * ct;
                        dt = TWORPI * lp_lon;
                        dt = dt + 1. / dt;
                        dt = sqrt(dt * dt - 4.);
                        if ((fabs(lp_lon) - HALFPI) < 0.) dt = -dt;
                        dt2 = dt * dt;
                        x1 = bt + ct; x1 *= x1;
                        t = bt + 3.*ct;
                        ft = x1 * (bt2 + ct2 * dt2 - 1.) + (1.-bt2) * (
                            bt2 * (t * t + 4. * ct2) +
                            ct2 * (12. * bt * ct + 4. * ct2) );
                        x1 = (dt*(x1 + ct2 - 1.) + 2.*sqrt(ft)) /
                            (4.* x1 + dt2);
                        xy_x = HALFPI * x1;
                        xy_y = HALFPI * sqrt(1. + dt * fabs(x1) - x1 * x1);
                        if (lp_lon < 0.) xy_x = -xy_x;
                        if (lp_lat < 0.) xy_y = -xy_y;
                    }
                }
            };

            // van der Grinten IV
            template <typename Parameters>
            void setup_vandg4(Parameters& par)
            {
                par.es = 0.;
                // par.fwd = s_forward;
            }

        }} // namespace detail::vandg4
    #endif // doxygen 

    /*!
        \brief van der Grinten IV projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Miscellaneous
         - Spheroid
         - no inverse
        \par Example
        \image html ex_vandg4.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct vandg4_spheroid : public detail::vandg4::base_vandg4_spheroid<Geographic, Cartesian, Parameters>
    {
        inline vandg4_spheroid(const Parameters& par) : detail::vandg4::base_vandg4_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::vandg4::setup_vandg4(this->m_par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class vandg4_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_f<vandg4_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void vandg4_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("vandg4", new vandg4_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail 
    #endif // doxygen

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_VANDG4_HPP


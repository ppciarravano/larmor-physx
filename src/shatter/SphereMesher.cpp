/*****************************************************************************
 * Larmor-Physx Version 1.0 2013
 * Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of Larmor-Physx (http://code.google.com/p/larmor-physx/).
 *
 * Larmor-Physx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Larmor-Physx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Larmor-Physx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 * $Id$
 *
 ****************************************************************************/

#include "SphereMesher.h"

#define RADIUS 1

GTFT sphere_function (Point_3S p) {
  const GTFT x2=p.x()*p.x(), y2=p.y()*p.y(), z2=p.z()*p.z();
  return x2+y2+z2 - RADIUS;
}

void ShereMesh() {
  Tr tr;            // 3D-Delaunay triangulation
  C2t3 c2t3 (tr);   // 2D-complex in 3D-Delaunay triangulation

  // defining the surface
  Surface_3 surface(sphere_function,             // pointer to function
                    Sphere_3(CGAL::ORIGIN, 2.)); // bounding sphere
  // Note that "2." above is the *squared* radius of the bounding sphere!

  // defining meshing criteria
  CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30.,  // angular bound
                                                     0.1,  // radius bound
                                                     0.1); // distance bound
  // meshing surface
  CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

  std::cout << "Final number of points: " << tr.number_of_vertices() << "\n";

  //Output mesh
  //Polyhedron polymesh;
  //bool result = CGAL::output_surface_facets_to_polyhedron(c2t3, polymesh);
  //std::cout << "output_surface_facets_to_polyhedron: " << result << "\n";

  //Scommentare per salvare mesh su file
  std::ofstream out("mesh_sphere_test_low.off");
  CGAL::output_surface_facets_to_off (out, c2t3);
  std::cout << "SAVED MESH\n";

  //std::list<TriangleGT> triangleMesh;
  //std::back_insert_iterator<std::list<TriangleGT> > bii(triangleMesh);
  //output_surface_facets_to_triangle_soup(c2t3, bii);
}





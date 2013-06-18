/*
 * Project name: Larmor-Physx
 * Released: 14 June 2013
 * Author: Pier Paolo Ciarravano
 * http://www.larmor.com
 *
 * License: This project is released under the Qt Public License (QPL - OSI-Approved Open Source license).
 * http://opensource.org/licenses/QPL-1.0
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 */

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





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

#ifndef SPHEREMESHER_H_
#define SPHEREMESHER_H_

#include <iostream>
#include <stdio.h>

#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>

//Output mesh
#include <fstream>
//#include <CGAL/IO/output_surface_facets_to_polyhedron.h>
//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Polyhedron_3.h>
//typedef CGAL::Simple_cartesian<double> KGT;
//typedef KGT::Triangle_3 TriangleGT;
//typedef CGAL::Polyhedron_3<KGT>         Polyhedron;
//#include <CGAL/IO/output_surface_facets_to_triangle_soup.h>
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h>

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;

// c2t3
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;

typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3S;
typedef GT::FT GTFT;

typedef GTFT (*Function)(Point_3S);

typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;

void ShereMesh();

#endif /* SPHEREMESHER_H_ */

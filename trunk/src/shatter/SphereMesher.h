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

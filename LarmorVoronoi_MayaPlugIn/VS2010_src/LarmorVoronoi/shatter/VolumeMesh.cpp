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

#include "DelaunayVoronoi.h"

#include <iostream>
#include <limits>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Implicit_mesh_domain_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/number_utils.h>
#include <CGAL/squared_distance_3.h>
#include <CGAL/Timer.h>
#include <CGAL/exceptions.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Polyhedron_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel KernelForPolyhedron;
typedef KernelForPolyhedron::FT FTKK;
typedef KernelForPolyhedron::Point_3 PointKK;
typedef CGAL::Polyhedron_3<KernelForPolyhedron>         Polyhedron;
typedef Polyhedron::HalfedgeDS           HalfedgeDS;
typedef CGAL::Polyhedral_mesh_domain_3<Polyhedron, KernelForPolyhedron> Mesh_domain;
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Trkr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Trkr> C3t3;
typedef CGAL::Mesh_criteria_3<Trkr> Mesh_criteria; //Criteria
typedef C3t3::Cells_in_complex_iterator Cell_iterator;
typedef Trkr::Point PointTetrahedra;


//Build_triangle_mesh for build the Polyhedron
template <class HDS>
class Build_triangle_mesh : public CGAL::Modifier_base<HDS> {

	TrianglesList *trianglesList;

	public:
	Build_triangle_mesh(TrianglesList& triangles) {
		trianglesList = &triangles;
	}

    void operator()( HDS& hds) {

    	int pointIndex = 0;

    	CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);
        B.begin_surface( 3*trianglesList->size(), trianglesList->size(), 0, CGAL::Polyhedron_incremental_builder_3<HDS>::ABSOLUTE_INDEXING);

       	//Insert all vertexes
    	std::list<Triangle>::iterator triangleIter;
		for(triangleIter = trianglesList->begin(); triangleIter != trianglesList->end(); ++triangleIter)
		{
			Triangle t = *triangleIter;
			for (int i = 0;  i < 3; ++i)
			{
				Point v = t.vertex(i);
				B.add_vertex( PointKK(CGAL::to_double(v.x()), CGAL::to_double(v.y()), CGAL::to_double(v.z())) );
				pointIndex++;
			}
		}

		//Insert faces
		pointIndex = 0;
		for(triangleIter = trianglesList->begin(); triangleIter != trianglesList->end(); ++triangleIter)
		{
			Triangle t = *triangleIter;
			B.begin_facet();
			for (int i = 0;  i < 3; ++i)
			{
				Point v = t.vertex(i);
				B.add_vertex_to_facet(pointIndex);
				pointIndex++;
			}
			B.end_facet();
		}

        B.end_surface();
    }
};

double minimumEdgeLength(TrianglesList &triangles) {

	double minimumEdge = std::numeric_limits<double>::max();

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = triangles.begin(); triangleIter != triangles.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		for (int i = 0;  i < 3; ++i)
		{
			Point v = t.vertex(i);
			//find the minimum edge
			FT distance = squared_distance(t.vertex(i),  t.vertex(i<2 ? i+1 : 0));
			double distanceDouble = CGAL::to_double(distance);
			if (distanceDouble < minimumEdge)
			{
				minimumEdge = distanceDouble;
				//std::cout << "  minimumEdge: " << minimumEdge << std::endl;
			}
		}
	}
	return minimumEdge;
}

double minimumBoundingDimension(TrianglesList &triangles) {

	MeshData meshData(triangles, TrianglesInfoList());
	std::pair<Point,Point> bb = getMeshBoundingBox(meshData);
	double xDimension = CGAL::to_double(bb.second.x()) - CGAL::to_double(bb.first.x());
	double yDimension = CGAL::to_double(bb.second.y()) - CGAL::to_double(bb.first.y());
	double zDimension = CGAL::to_double(bb.second.z()) - CGAL::to_double(bb.first.z());
	std::cout << "  minimumBoundingDimension: " << xDimension << ", "<< yDimension << ", " << zDimension << std::endl;
	double result = xDimension;
	if (yDimension > result)
	{
		result = yDimension;
	}
	if (zDimension > result)
	{
		result = zDimension;
	}
	return result;
}

//The Inertia is refer to a mass with density = 1.0, se we have to multiply the inertia result of this function with density.
// For Inertia: http://www.ce.utexas.edu/prof/tonon/Publications/Papers/Paper%20P%2032,%20Explicit%20exact%20formulas%20for...pdf
// For Volume: http://amp.ece.cmu.edu/Publication/Cha/icip01_Cha.pdf
//Return std::vector<double> result = {volume, inertia_a , inertia_b, inertia_c, inertia_aa, inertia_bb, inertia_cc}
std::vector<double> internalVolumeAndInertiaMesh(TrianglesList &triangles, double facetDistance) {

	CGAL::Timer timer;
	timer.start();

	FTKK volumeTotal = 0.0;

	FTKK inertia_a = 0.0;
	FTKK inertia_b = 0.0;
	FTKK inertia_c = 0.0;
	FTKK inertia_aa = 0.0;
	FTKK inertia_bb = 0.0;
	FTKK inertia_cc = 0.0;

	//Inertia tensor is:
	// ( (inertia_a, -inertia_bb, -inertia_cc),
	//   (-inertia_bb, inertia_b, -inertia_aa),
	//   (-inertia_cc, -inertia_aa, inertia_c) )

	try
	{
		Polyhedron P;

		Build_triangle_mesh<HalfedgeDS> triangle(triangles);
		P.delegate(triangle);
		std::cout << "Polyhedron is_pure_triangle: " << P.is_pure_triangle() << std::endl;
		//std::cout << "Polyhedron is_closed: " << P.is_closed() << std::endl;

		//Volumetric Mesh
		Mesh_domain domain(P);

		//Mesh criteria
		// See http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Mesh_3_ref/Class_Mesh_criteria_3.html
		//Mesh_criteria criteria(CGAL::parameters::facet_angle=30, CGAL::parameters::facet_size=1.1, CGAL::parameters::facet_distance=1.025,
		//		CGAL::parameters::facet_topology = CGAL::FACET_VERTICES_ON_SURFACE, CGAL::parameters::cell_radius_edge_ratio=2, CGAL::parameters::cell_size=0.5);
		//Mesh_criteria criteria(CGAL::parameters::cell_radius_edge_ratio=0.5, CGAL::parameters::cell_size=0.4);
		//Mesh_criteria criteria(CGAL::parameters::facet_angle = 5, CGAL::parameters::facet_size=0.5, ....
		//facet_topology = FACET_VERTICES_ON_SURFACE, FACET_VERTICES_ON_SAME_SURFACE_PATCH, FACET_VERTICES_ON_SAME_SURFACE_PATCH_WITH_ADJACENCY_CHECK};
		//Mesh_criteria criteria(CGAL::parameters::cell_size = 0.1,
		//		CGAL::parameters::facet_topology = CGAL::FACET_VERTICES_ON_SAME_SURFACE_PATCH_WITH_ADJACENCY_CHECK);
		Mesh_criteria criteria(CGAL::parameters::facet_distance = facetDistance,
				CGAL::parameters::facet_topology = CGAL::FACET_VERTICES_ON_SAME_SURFACE_PATCH_WITH_ADJACENCY_CHECK);

		//Mesh generation
		//C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);
		C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, CGAL::parameters::perturb(), CGAL::parameters::exude());

		std::cout << "Number of Tetrahedra: " << c3t3.number_of_cells_in_complex() << std::endl;
		for( Cell_iterator cit = c3t3.cells_in_complex_begin(); cit != c3t3.cells_in_complex_end(); ++cit )
		{
			PointTetrahedra p[4];
			for (int i=0; i<4; i++)
			{
				p[i] = cit->vertex(i)->point();
				p[i] = PointTetrahedra(p[i].x(), p[i].y(), p[i].z());
			}

			//Volume Calculation
			FTKK W = CGAL::squared_distance(p[0], p[1]);
			FTKK U = CGAL::squared_distance(p[0], p[2]);
			FTKK v = CGAL::squared_distance(p[0], p[3]);
			FTKK V = CGAL::squared_distance(p[1], p[2]);
			FTKK u = CGAL::squared_distance(p[1], p[3]);
			FTKK w = CGAL::squared_distance(p[2], p[3]);

			FTKK volumeSquared = 4*u*v*w - u*CGAL::square(v+w-U) - v*CGAL::square(w+u-V) -
					w*CGAL::square(u+v-W) +	(v+w-U)*(w+u-V)*(u+v-W);

			FTKK volumeTet = CGAL::sqrt(volumeSquared) / 12;
			volumeTotal += volumeTet;

			//Inertia tensor Calculation
			/*
			FTKK inertia_a_tet = ( y1*y1 + y1*y2 + y2*y2 + y1*y3 + y2*y3
				+ y3*y3 + y1*y4 + y2*y4 + y3*y4 + y4*y4 +z1*z1 + z1*z2
				+ z2*z2 +z1*z3 + z2*z3 + z3*z3 + z1*z4 + z2*z4 + z3*z4 + z4*z4 )
				* volumeTet * 6 / 60;
			FTKK inertia_b_tet = ( x1*x1 + x1*x2 + x2*x2 + x1*x3 + x2*x3
				+ x3*x3 + x1*x4 + x2*x4 + x3*x4 + x4*x4 +z1*z1 + z1*z2
				+ z2*z2 +z1*z3 + z2*z3 + z3*z3 + z1*z4 + z2*z4 + z3*z4 + z4*z4 )
				* volumeTet * 6 / 60;
			FTKK inertia_c_tet = ( x1*x1 + x1*x2 + x2*x2 + x1*x3 + x2*x3
				+ x3*x3 + x1*x4 + x2*x4 + x3*x4 + x4*x4 +y1*y1 + y1*y2
				+ y2*y2 +y1*y3 + y2*y3 + y3*y3 + y1*y4 + y2*y4 + y3*y4 + y4*y4 )
				* volumeTet * 6 / 60;
			FTKK inertia_aa_tet = ( 2*y1*z1 + y2*z1 + y3*z1 + y4*z1 + y1*z2
				+ 2*y2*z2 + y3*z2 + y4*z2 + y1*z3 + y2*z3 + 2*y3*z3
				+ y4*z3 + y1*z4 + y2*z4 + y3*z4 + 2*y4*z4 )
				* volumeTet * 6 / 120;
			FTKK inertia_bb_tet = ( 2*x1*z1 + x2*z1 + x3*z1 + x4*z1 + x1*z2
				+ 2*x2*z2 + x3*z2 + x4*z2 + x1*z3 + x2*z3 + 2*x3*z3
				+ x4*z3 + x1*z4 + x2*z4 + x3*z4 + 2*x4*z4 )
				* volumeTet * 6 / 120;
			FTKK inertia_cc_tet = ( 2*x1*y1 + x2*y1 + x3*y1 + x4*y1 + x1*y2
				+ 2*x2*y2 + x3*y2 + x4*y2 + x1*y3 + x2*y3 + 2*x3*y3
				+ x4*y3 + x1*y4 + x2*y4 + x3*y4 + 2*x4*y4 )
				* volumeTet * 6 / 120;
			*/

			FTKK inertia_a_tet = ( p[0].y()*p[0].y() + p[0].y()*p[1].y() + p[1].y()*p[1].y() + p[0].y()*p[2].y() + p[1].y()*p[2].y()
				+ p[2].y()*p[2].y() + p[0].y()*p[3].y() + p[1].y()*p[3].y() + p[2].y()*p[3].y() + p[3].y()*p[3].y() +p[0].z()*p[0].z() + p[0].z()*p[1].z()
				+ p[1].z()*p[1].z() +p[0].z()*p[2].z() + p[1].z()*p[2].z() + p[2].z()*p[2].z() + p[0].z()*p[3].z() + p[1].z()*p[3].z() + p[2].z()*p[3].z() + p[3].z()*p[3].z() )
				* volumeTet * 6 / 60;
			FTKK inertia_b_tet = ( p[0].x()*p[0].x() + p[0].x()*p[1].x() + p[1].x()*p[1].x() + p[0].x()*p[2].x() + p[1].x()*p[2].x()
				+ p[2].x()*p[2].x() + p[0].x()*p[3].x() + p[1].x()*p[3].x() + p[2].x()*p[3].x() + p[3].x()*p[3].x() +p[0].z()*p[0].z() + p[0].z()*p[1].z()
				+ p[1].z()*p[1].z() +p[0].z()*p[2].z() + p[1].z()*p[2].z() + p[2].z()*p[2].z() + p[0].z()*p[3].z() + p[1].z()*p[3].z() + p[2].z()*p[3].z() + p[3].z()*p[3].z() )
				* volumeTet * 6 / 60;
			FTKK inertia_c_tet = ( p[0].x()*p[0].x() + p[0].x()*p[1].x() + p[1].x()*p[1].x() + p[0].x()*p[2].x() + p[1].x()*p[2].x()
				+ p[2].x()*p[2].x() + p[0].x()*p[3].x() + p[1].x()*p[3].x() + p[2].x()*p[3].x() + p[3].x()*p[3].x() +p[0].y()*p[0].y() + p[0].y()*p[1].y()
				+ p[1].y()*p[1].y() +p[0].y()*p[2].y() + p[1].y()*p[2].y() + p[2].y()*p[2].y() + p[0].y()*p[3].y() + p[1].y()*p[3].y() + p[2].y()*p[3].y() + p[3].y()*p[3].y() )
				* volumeTet * 6 / 60;
			FTKK inertia_aa_tet = ( 2*p[0].y()*p[0].z() + p[1].y()*p[0].z() + p[2].y()*p[0].z() + p[3].y()*p[0].z() + p[0].y()*p[1].z()
				+ 2*p[1].y()*p[1].z() + p[2].y()*p[1].z() + p[3].y()*p[1].z() + p[0].y()*p[2].z() + p[1].y()*p[2].z() + 2*p[2].y()*p[2].z()
				+ p[3].y()*p[2].z() + p[0].y()*p[3].z() + p[1].y()*p[3].z() + p[2].y()*p[3].z() + 2*p[3].y()*p[3].z() )
				* volumeTet * 6 / 120;
			FTKK inertia_bb_tet = ( 2*p[0].x()*p[0].z() + p[1].x()*p[0].z() + p[2].x()*p[0].z() + p[3].x()*p[0].z() + p[0].x()*p[1].z()
				+ 2*p[1].x()*p[1].z() + p[2].x()*p[1].z() + p[3].x()*p[1].z() + p[0].x()*p[2].z() + p[1].x()*p[2].z() + 2*p[2].x()*p[2].z()
				+ p[3].x()*p[2].z() + p[0].x()*p[3].z() + p[1].x()*p[3].z() + p[2].x()*p[3].z() + 2*p[3].x()*p[3].z() )
				* volumeTet * 6 / 120;
			FTKK inertia_cc_tet = ( 2*p[0].x()*p[0].y() + p[1].x()*p[0].y() + p[2].x()*p[0].y() + p[3].x()*p[0].y() + p[0].x()*p[1].y()
				+ 2*p[1].x()*p[1].y() + p[2].x()*p[1].y() + p[3].x()*p[1].y() + p[0].x()*p[2].y() + p[1].x()*p[2].y() + 2*p[2].x()*p[2].y()
				+ p[3].x()*p[2].y() + p[0].x()*p[3].y() + p[1].x()*p[3].y() + p[2].x()*p[3].y() + 2*p[3].x()*p[3].y() )
				* volumeTet * 6 / 120;

			inertia_a += inertia_a_tet;
			inertia_b += inertia_b_tet;
			inertia_c += inertia_c_tet;
			inertia_aa += inertia_aa_tet;
			inertia_bb += inertia_bb_tet;
			inertia_cc += inertia_cc_tet;

		}
		std::cout << "VolumeTotal: " << volumeTotal << std::endl;

		std::cout << "inertia_a: " << inertia_a << std::endl;
		std::cout << "inertia_b: " << inertia_b << std::endl;
		std::cout << "inertia_c: " << inertia_c << std::endl;
		std::cout << "inertia_aa: " << inertia_aa << std::endl;
		std::cout << "inertia_bb: " << inertia_bb << std::endl;
		std::cout << "inertia_cc: " << inertia_cc << std::endl;

	}
	catch (CGAL::Assertion_exception e)
	{
		std::cout << "ERROR: internalVolumeAndInertiaMesh CGAL::Assertion_exception" << e.message() << std::endl;
		volumeTotal = 0.0;
	}

	timer.stop();
	std::cout << "Total VolumeTotal time: " << timer.time() << std::endl;

	std::vector<double> result(7);
	result.at(0) = CGAL::to_double(volumeTotal);
	result.at(1) = CGAL::to_double(inertia_a);
	result.at(2) = CGAL::to_double(inertia_b);
	result.at(3) = CGAL::to_double(inertia_c);
	result.at(4) = CGAL::to_double(inertia_aa);
	result.at(5) = CGAL::to_double(inertia_bb);
	result.at(6) = CGAL::to_double(inertia_cc);

	return result;
}


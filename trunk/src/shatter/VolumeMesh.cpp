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

// Return the volume and the inertia tensor relative to the origin (0.0, 0.0, 0.0) of a tetrahedra
// Algorithms used:
// [1] For Inertia: http://www.ce.utexas.edu/prof/tonon/Publications/Papers/Paper%20P%2032,%20Explicit%20exact%20formulas%20for...pdf
//   (Explicit exact formulas for the 3-D tetrahedron inertia tensor in terms of its vertex coordinates - F.Tonon )
// [2] For Volume: http://amp.ece.cmu.edu/Publication/Cha/icip01_Cha.pdf
//   (Efficient feature extraction for 2d/3d objects in mesh representation - C.Zhang and T.Chen)
//
// To return the inertia tensor relative to the centroid we have to translate the tetrahedra vertices: p[n] = p[n] - centroid
//
// To test: for the tetrahedra with vertices:
//    pt[0] = PT(8.33220, -11.86875, 0.93355)
//    pt[1] = PT(0.75523 ,5.00000, 16.37072)
//    pt[2] = PT(52.61236, 5.00000, -5.38580)
//    pt[3] = PT(2.00000, 5.00000, 3.00000)
// centered in its centroid PT(15.92492, 0.78281, 3.732962)
// the algorithm returns correctly like the numerical test example of [1] the values:
//    VolumeTotal: 1873.23
//    inertia_a:   43520.3
//    inertia_b:   194711.0
//    inertia_c:   191169.0
//    inertia_aa:  4417.66
//    inertia_bb:  -46343.2
//    inertia_cc:  11996.2
//
template <class FT, class PT>
void volumeAndInertiaTetrahedra(PT p[], FT volumeAndInertia[]) {

	//volumeTotal <---> volumeAndInertia[0]
	//inertia_a <---> volumeAndInertia[1]
	//inertia_b <---> volumeAndInertia[2]
	//inertia_c <---> volumeAndInertia[3]
	//inertia_aa <---> volumeAndInertia[4]
	//inertia_bb <---> volumeAndInertia[5]
	//inertia_cc <---> volumeAndInertia[6]

	//Volume Calculation
	FT W = CGAL::squared_distance(p[0], p[1]);
	FT U = CGAL::squared_distance(p[0], p[2]);
	FT v = CGAL::squared_distance(p[0], p[3]);
	FT V = CGAL::squared_distance(p[1], p[2]);
	FT u = CGAL::squared_distance(p[1], p[3]);
	FT w = CGAL::squared_distance(p[2], p[3]);

	FT volumeSquared = 4*u*v*w - u*CGAL::square(v+w-U) - v*CGAL::square(w+u-V) -
			w*CGAL::square(u+v-W) +	(v+w-U)*(w+u-V)*(u+v-W);

	FT volumeTet = CGAL::sqrt(volumeSquared) / 12;
	volumeAndInertia[0] += volumeTet;

	//Inertia tensor Calculation

	//To return the inertia tensor relative to the tetrahedron centroid uncomment the block bellow:
	// if the block bellow is uncommented then return the inertia tensor relative to the origin shifting the inertia tensor
	// using the Parallel axis theorem (Huygens–Steiner theorem - http://en.wikipedia.org/wiki/Parallel_axis_theorem)
	/*
	//Translate the tetrahedron in its centroid
	PT centr = PT( (p[0].x() + p[1].x() + p[2].x() + p[3].x()) / 4.0,
				(p[0].y() + p[1].y() + p[2].y() + p[3].y()) / 4.0,
				(p[0].z() + p[1].z() + p[2].z() + p[3].z()) / 4.0 );
	p[0] = PT(p[0].x() - centr.x(), p[0].y() - centr.y(), p[0].z() - centr.z());
	p[1] = PT(p[1].x() - centr.x(), p[1].y() - centr.y(), p[1].z() - centr.z());
	p[2] = PT(p[2].x() - centr.x(), p[2].y() - centr.y(), p[2].z() - centr.z());
	p[3] = PT(p[3].x() - centr.x(), p[3].y() - centr.y(), p[3].z() - centr.z());
	*/

	/*
	FT inertia_a_tet = ( y1*y1 + y1*y2 + y2*y2 + y1*y3 + y2*y3
		+ y3*y3 + y1*y4 + y2*y4 + y3*y4 + y4*y4 +z1*z1 + z1*z2
		+ z2*z2 +z1*z3 + z2*z3 + z3*z3 + z1*z4 + z2*z4 + z3*z4 + z4*z4 )
		* volumeTet * 6 / 60;
	FT inertia_b_tet = ( x1*x1 + x1*x2 + x2*x2 + x1*x3 + x2*x3
		+ x3*x3 + x1*x4 + x2*x4 + x3*x4 + x4*x4 +z1*z1 + z1*z2
		+ z2*z2 +z1*z3 + z2*z3 + z3*z3 + z1*z4 + z2*z4 + z3*z4 + z4*z4 )
		* volumeTet * 6 / 60;
	FT inertia_c_tet = ( x1*x1 + x1*x2 + x2*x2 + x1*x3 + x2*x3
		+ x3*x3 + x1*x4 + x2*x4 + x3*x4 + x4*x4 +y1*y1 + y1*y2
		+ y2*y2 +y1*y3 + y2*y3 + y3*y3 + y1*y4 + y2*y4 + y3*y4 + y4*y4 )
		* volumeTet * 6 / 60;
	FT inertia_aa_tet = ( 2*y1*z1 + y2*z1 + y3*z1 + y4*z1 + y1*z2
		+ 2*y2*z2 + y3*z2 + y4*z2 + y1*z3 + y2*z3 + 2*y3*z3
		+ y4*z3 + y1*z4 + y2*z4 + y3*z4 + 2*y4*z4 )
		* volumeTet * 6 / 120;
	FT inertia_bb_tet = ( 2*x1*z1 + x2*z1 + x3*z1 + x4*z1 + x1*z2
		+ 2*x2*z2 + x3*z2 + x4*z2 + x1*z3 + x2*z3 + 2*x3*z3
		+ x4*z3 + x1*z4 + x2*z4 + x3*z4 + 2*x4*z4 )
		* volumeTet * 6 / 120;
	FT inertia_cc_tet = ( 2*x1*y1 + x2*y1 + x3*y1 + x4*y1 + x1*y2
		+ 2*x2*y2 + x3*y2 + x4*y2 + x1*y3 + x2*y3 + 2*x3*y3
		+ x4*y3 + x1*y4 + x2*y4 + x3*y4 + 2*x4*y4 )
		* volumeTet * 6 / 120;
	*/

	FT inertia_a_tet = ( p[0].y()*p[0].y() + p[0].y()*p[1].y() + p[1].y()*p[1].y() + p[0].y()*p[2].y() + p[1].y()*p[2].y()
		+ p[2].y()*p[2].y() + p[0].y()*p[3].y() + p[1].y()*p[3].y() + p[2].y()*p[3].y() + p[3].y()*p[3].y() +p[0].z()*p[0].z() + p[0].z()*p[1].z()
		+ p[1].z()*p[1].z() +p[0].z()*p[2].z() + p[1].z()*p[2].z() + p[2].z()*p[2].z() + p[0].z()*p[3].z() + p[1].z()*p[3].z() + p[2].z()*p[3].z() + p[3].z()*p[3].z() )
		* volumeTet * 6.0 / 60.0;
	FT inertia_b_tet = ( p[0].x()*p[0].x() + p[0].x()*p[1].x() + p[1].x()*p[1].x() + p[0].x()*p[2].x() + p[1].x()*p[2].x()
		+ p[2].x()*p[2].x() + p[0].x()*p[3].x() + p[1].x()*p[3].x() + p[2].x()*p[3].x() + p[3].x()*p[3].x() +p[0].z()*p[0].z() + p[0].z()*p[1].z()
		+ p[1].z()*p[1].z() +p[0].z()*p[2].z() + p[1].z()*p[2].z() + p[2].z()*p[2].z() + p[0].z()*p[3].z() + p[1].z()*p[3].z() + p[2].z()*p[3].z() + p[3].z()*p[3].z() )
		* volumeTet * 6.0 / 60.0;
	FT inertia_c_tet = ( p[0].x()*p[0].x() + p[0].x()*p[1].x() + p[1].x()*p[1].x() + p[0].x()*p[2].x() + p[1].x()*p[2].x()
		+ p[2].x()*p[2].x() + p[0].x()*p[3].x() + p[1].x()*p[3].x() + p[2].x()*p[3].x() + p[3].x()*p[3].x() +p[0].y()*p[0].y() + p[0].y()*p[1].y()
		+ p[1].y()*p[1].y() +p[0].y()*p[2].y() + p[1].y()*p[2].y() + p[2].y()*p[2].y() + p[0].y()*p[3].y() + p[1].y()*p[3].y() + p[2].y()*p[3].y() + p[3].y()*p[3].y() )
		* volumeTet * 6.0 / 60.0;
	FT inertia_aa_tet = ( 2.0*p[0].y()*p[0].z() + p[1].y()*p[0].z() + p[2].y()*p[0].z() + p[3].y()*p[0].z() + p[0].y()*p[1].z()
		+ 2.0*p[1].y()*p[1].z() + p[2].y()*p[1].z() + p[3].y()*p[1].z() + p[0].y()*p[2].z() + p[1].y()*p[2].z() + 2.0*p[2].y()*p[2].z()
		+ p[3].y()*p[2].z() + p[0].y()*p[3].z() + p[1].y()*p[3].z() + p[2].y()*p[3].z() + 2.0*p[3].y()*p[3].z() )
		* volumeTet * 6.0 / 120.0;
	FT inertia_bb_tet = ( 2.0*p[0].x()*p[0].z() + p[1].x()*p[0].z() + p[2].x()*p[0].z() + p[3].x()*p[0].z() + p[0].x()*p[1].z()
		+ 2.0*p[1].x()*p[1].z() + p[2].x()*p[1].z() + p[3].x()*p[1].z() + p[0].x()*p[2].z() + p[1].x()*p[2].z() + 2.0*p[2].x()*p[2].z()
		+ p[3].x()*p[2].z() + p[0].x()*p[3].z() + p[1].x()*p[3].z() + p[2].x()*p[3].z() + 2.0*p[3].x()*p[3].z() )
		* volumeTet * 6.0 / 120.0;
	FT inertia_cc_tet = ( 2.0*p[0].x()*p[0].y() + p[1].x()*p[0].y() + p[2].x()*p[0].y() + p[3].x()*p[0].y() + p[0].x()*p[1].y()
		+ 2.0*p[1].x()*p[1].y() + p[2].x()*p[1].y() + p[3].x()*p[1].y() + p[0].x()*p[2].y() + p[1].x()*p[2].y() + 2.0*p[2].x()*p[2].y()
		+ p[3].x()*p[2].y() + p[0].x()*p[3].y() + p[1].x()*p[3].y() + p[2].x()*p[3].y() + 2.0*p[3].x()*p[3].y() )
		* volumeTet * 6.0 / 120.0;

	volumeAndInertia[1] += inertia_a_tet;
	volumeAndInertia[2] += inertia_b_tet;
	volumeAndInertia[3] += inertia_c_tet;
	volumeAndInertia[4] += inertia_aa_tet;
	volumeAndInertia[5] += inertia_bb_tet;
	volumeAndInertia[6] += inertia_cc_tet;

}

//The Inertia is refer to a mass with uniform density = 1.0, so we have to multiply the inertia result from this function with density.
//Return std::vector<double> result = {volume, inertia_a , inertia_b, inertia_c, inertia_aa, inertia_bb, inertia_cc}
//The inertia tensor is relative to the origin (0.0, 0.0, 0.0)
std::vector<double> internalVolumeAndInertiaMesh(TrianglesList &triangles, double facetDistance) {

	CGAL::Timer timer;
	timer.start();

	//Init output values
	FTKK volumeAndInertia[7];
	volumeAndInertia[0] = 0.0; //volumeTotal
	volumeAndInertia[1] = 0.0; //inertia_a
	volumeAndInertia[2] = 0.0; //inertia_b
	volumeAndInertia[3] = 0.0; //inertia_c
	volumeAndInertia[4] = 0.0; //inertia_aa
	volumeAndInertia[5] = 0.0; //inertia_bb
	volumeAndInertia[6] = 0.0; //inertia_cc

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
			}

			//Calcolate volume and inertia of the Tetrahedra
			volumeAndInertiaTetrahedra(p, volumeAndInertia);
		}

		std::cout << "Non-Convex VolumeTotal: " << volumeAndInertia[0] << std::endl;
		std::cout << "inertia_a:   " << volumeAndInertia[1] << std::endl;
		std::cout << "inertia_b:   " << volumeAndInertia[2] << std::endl;
		std::cout << "inertia_c:   " << volumeAndInertia[3] << std::endl;
		std::cout << "inertia_aa:  " << volumeAndInertia[4] << std::endl;
		std::cout << "inertia_bb:  " << volumeAndInertia[5] << std::endl;
		std::cout << "inertia_cc:  " << volumeAndInertia[6] << std::endl;

	}
	catch (CGAL::Assertion_exception e)
	{
		std::cout << "ERROR: internalVolumeAndInertiaMesh CGAL::Assertion_exception" << e.message() << std::endl;
		volumeAndInertia[0] = 0.0; //volumeTotal
		volumeAndInertia[1] = 0.0; //inertia_a
		volumeAndInertia[2] = 0.0; //inertia_b
		volumeAndInertia[3] = 0.0; //inertia_c
		volumeAndInertia[4] = 0.0; //inertia_aa
		volumeAndInertia[5] = 0.0; //inertia_bb
		volumeAndInertia[6] = 0.0; //inertia_cc
	}

	timer.stop();
	std::cout << "Total VolumeTotal time: " << timer.time() << std::endl;

	std::vector<double> result(7);
	result.at(0) = CGAL::to_double(volumeAndInertia[0]);
	result.at(1) = CGAL::to_double(volumeAndInertia[1]);
	result.at(2) = CGAL::to_double(volumeAndInertia[2]);
	result.at(3) = CGAL::to_double(volumeAndInertia[3]);
	result.at(4) = CGAL::to_double(volumeAndInertia[4]);
	result.at(5) = CGAL::to_double(volumeAndInertia[5]);
	result.at(6) = CGAL::to_double(volumeAndInertia[6]);

	return result;
}

//The Inertia is refer to a mass with uniform density = 1.0, so we have to multiply the inertia result from this function with density.
//Return std::vector<double> result = {volume, inertia_a , inertia_b, inertia_c, inertia_aa, inertia_bb, inertia_cc}
//The inertia tensor is relative to the origin (0.0, 0.0, 0.0)
std::vector<double> internalVolumeAndInertiaMeshConvex(TrianglesList &triangles, Point meshCenter) {

	CGAL::Timer timer;
	timer.start();

	//Init output values
	FTKK volumeAndInertia[7];
	volumeAndInertia[0] = 0.0; //volumeTotal
	volumeAndInertia[1] = 0.0; //inertia_a
	volumeAndInertia[2] = 0.0; //inertia_b
	volumeAndInertia[3] = 0.0; //inertia_c
	volumeAndInertia[4] = 0.0; //inertia_aa
	volumeAndInertia[5] = 0.0; //inertia_bb
	volumeAndInertia[6] = 0.0; //inertia_cc

	//for each mesh Tetrahedra calculate the volume and inertia tensor
	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = triangles.begin(); triangleIter != triangles.end(); ++triangleIter)
	{
		PointKK p[4];
		Triangle t = *triangleIter;
		for (int i = 0;  i < 3; ++i)
		{
			p[i] = PointKK(CGAL::to_double(t.vertex(i).x()),
							CGAL::to_double(t.vertex(i).y()),
							CGAL::to_double(t.vertex(i).z()));
		}

		p[3] = PointKK(CGAL::to_double(meshCenter.x()),
						CGAL::to_double(meshCenter.y()),
						CGAL::to_double(meshCenter.z()));

		//Calcolate volume and inertia of the Tetrahedra
		volumeAndInertiaTetrahedra(p, volumeAndInertia);
	}

	std::cout << "Convex VolumeTotal: " << volumeAndInertia[0] << std::endl;
	std::cout << "inertia_a:   " << volumeAndInertia[1] << std::endl;
	std::cout << "inertia_b:   " << volumeAndInertia[2] << std::endl;
	std::cout << "inertia_c:   " << volumeAndInertia[3] << std::endl;
	std::cout << "inertia_aa:  " << volumeAndInertia[4] << std::endl;
	std::cout << "inertia_bb:  " << volumeAndInertia[5] << std::endl;
	std::cout << "inertia_cc:  " << volumeAndInertia[6] << std::endl;

	timer.stop();
	std::cout << "Total VolumeTotal convex time: " << timer.time() << std::endl;

	std::vector<double> result(7);
	result.at(0) = CGAL::to_double(volumeAndInertia[0]);
	result.at(1) = CGAL::to_double(volumeAndInertia[1]);
	result.at(2) = CGAL::to_double(volumeAndInertia[2]);
	result.at(3) = CGAL::to_double(volumeAndInertia[3]);
	result.at(4) = CGAL::to_double(volumeAndInertia[4]);
	result.at(5) = CGAL::to_double(volumeAndInertia[5]);
	result.at(6) = CGAL::to_double(volumeAndInertia[6]);

	return result;
}



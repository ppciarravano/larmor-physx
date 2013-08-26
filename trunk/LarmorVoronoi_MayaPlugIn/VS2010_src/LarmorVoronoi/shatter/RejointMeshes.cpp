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
#include <vector>
#include <list>
#include <map>
#include <set>

//For Box intersection
#include <CGAL/intersections.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/box_intersection_d.h>
#include <CGAL/function_objects.h>
#include <CGAL/algorithm.h>


struct TriangleVisitedInfoRM {

	Triangle *triangle;
	TriangleInfo *triangleInfo;
	bool toRemove;

	//disjoint mesh index
	short int disjointMeshIndex;

};

//For Box Collision
typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TriangleVisitedInfoRM*> BoxInt;

int totalchecks = 0;
void reportAdjacentIntersectionCallback(const BoxInt& b1, const BoxInt& b2) {

	TriangleVisitedInfoRM *m1 = b1.handle();
	TriangleVisitedInfoRM *m2 = b2.handle();

	if (m1->disjointMeshIndex != m2->disjointMeshIndex)
	{
		//std::cout << m1->disjointMeshIndex << " - " << m2->disjointMeshIndex << std::endl;
		totalchecks++;
		//Simple intersection: if (CGAL::do_intersect( *(m1->triangle), *(m2->triangle) ) )
		// only discard the couple of triangles if their intersection is a triangle, so their are complanar and touching
		//if (  (*(m1->triangle)).supporting_plane() == (*(m1->triangle)).supporting_plane() ) // This could have errors

		//CGAL::Object intersectionResult = CGAL::intersection(*(m1->triangle), *(m2->triangle));
		//if (const CGAL::Triangle_3<Kernel> *itriangle = CGAL::object_cast<CGAL::Triangle_3<Kernel> >(&intersectionResult))

		if (  (*(m1->triangle)).supporting_plane() == (*(m1->triangle)).supporting_plane() ) // This could have errors but actually it works better than the previous condition
		{
			//std::cout << "INTERSECT section is a Triangle!!" << std::endl;
			m1->toRemove = true;
			m2->toRemove = true;
		}
	}
}

//TODO:
//Completely change this algorithm: see voronoiShatter function in DelaunayVoronoi.cpp
// Change TriangleInfo adding:
// int meshId, meshIdAdjacent
// and populate these new two variable for the triangles into the cut plane from the voronoiShatter function;
// each mesh piece will have a different id;
// these two new variable could be used into the RejointMeshes algorithm instead of reportAdjacentIntersectionCallback:
// knowing this ids it is very easy remove the the triangles in shared cut surfaces

// Rejoint meshes removing the triangles in shared cut surfaces
MeshData rejointMeshes(std::list<MeshData> &listMeshData) {

	std::cout << "Executing rejointMeshes algorithm..." << std::endl;

	CGAL::Timer timer;
	timer.start();

	TrianglesList resultTriangleList;
	TrianglesInfoList resultTrianglesInfoList;

	//Collision boxes
	std::vector<BoxInt> boxes;

	std::list<MeshData>::iterator meshDataInter;
	short int meshIndex = 0;
	for(meshDataInter = listMeshData.begin(); meshDataInter != listMeshData.end(); ++meshDataInter)
	{
		std::list<Triangle>::iterator triangleIter;
		std::list<TriangleInfo>::iterator triangleInfoIter;
		for(triangleIter = meshDataInter->first.begin(), triangleInfoIter = meshDataInter->second.begin();
					triangleIter != meshDataInter->first.end();
					++triangleIter, ++triangleInfoIter)
		{
			Triangle *tp = &(*triangleIter); //Use the pointer, it should not change into the container, since there aren't new added elements
			TriangleInfo *tip = &(*triangleInfoIter); //Use the pointer, it should not change into the container, since there aren't new added elements
			if (!(tp->is_degenerate()))
			{
				if (tip->cutType == CUTTERMESHER_TRIANGLE_IN_CUT_PLANE)
				{
					TriangleVisitedInfoRM *tvi = new TriangleVisitedInfoRM;
					tvi->triangle = tp;
					tvi->triangleInfo = tip;
					tvi->toRemove = false;
					tvi->disjointMeshIndex = meshIndex;
					boxes.push_back( BoxInt( tvi->triangle->bbox(), tvi ));
				}
				else
				{
					//Aggiungo direttamente i triangoli della frontiera alla mesh di result
					resultTriangleList.push_back(*tp);
					resultTrianglesInfoList.push_back(*tip);
				}
			}
		}

		meshIndex++;
	}
	std::cout << " rejointMeshes checking intersections of boxes.size: " << boxes.size() << std::endl;


	//Do intersection    CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), reportAdjacentIntersectionCallback);
    std::cout << " rejointMeshes total box checks: " << totalchecks << std::endl;

	//cycle on boxes and build result and delete tvi
	std::vector<BoxInt>::iterator vectorBoxesIter;
	for(vectorBoxesIter = boxes.begin(); vectorBoxesIter != boxes.end(); ++vectorBoxesIter)
	{
		BoxInt boxInt = *vectorBoxesIter;
		TriangleVisitedInfoRM *tviHandled = boxInt.handle();

		if (!(tviHandled->toRemove))
		{
			//Se il triangolo non e' mai stato in intersezione complanare con un altro triangolo allora lo aggiungo in output
			resultTriangleList.push_back(*(tviHandled->triangle));
			resultTrianglesInfoList.push_back(*(tviHandled->triangleInfo));
		}

		delete tviHandled;
	}

	timer.stop();
	std::cout << "Total rejointMeshes time: " << timer.time() << std::endl;

	return MeshData(resultTriangleList, resultTrianglesInfoList);

}


// Rejoint meshes but it doesn't remove the triangles in shared cut surfaces
MeshData simpleRejointMeshes(std::list<MeshData> &listMeshData) {

	std::cout << "Executing simpleRejointMeshes algorithm..." << std::endl;

	CGAL::Timer timer;
	timer.start();

	TrianglesList resultTriangleList;
	TrianglesInfoList resultTrianglesInfoList;

	std::list<MeshData>::iterator meshDataInter;
	for(meshDataInter = listMeshData.begin(); meshDataInter != listMeshData.end(); ++meshDataInter)
	{
		std::list<Triangle>::iterator triangleIter;
		std::list<TriangleInfo>::iterator triangleInfoIter;
		for(triangleIter = meshDataInter->first.begin(), triangleInfoIter = meshDataInter->second.begin();
					triangleIter != meshDataInter->first.end();
					++triangleIter, ++triangleInfoIter)
		{

			resultTriangleList.push_back(*triangleIter);
			resultTrianglesInfoList.push_back(*triangleInfoIter);

		}
	}

	timer.stop();
	std::cout << "Total simpleRejointMeshes time: " << timer.time() << std::endl;

	return MeshData(resultTriangleList, resultTrianglesInfoList);

}



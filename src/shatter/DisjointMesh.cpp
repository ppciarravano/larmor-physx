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
#include <boost/unordered_map.hpp>

//For Box intersection
#include <CGAL/intersections.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/box_intersection_d.h>
#include <CGAL/function_objects.h>
#include <CGAL/Join_input_iterator.h>
#include <CGAL/algorithm.h>
#include <boost/unordered_set.hpp>
#include <boost/thread.hpp>

#include <CGAL/Aff_transformation_2.h>

#define DISJOINTMESH_LOG

typedef std::list<unsigned int> ListVisitedInfoIndex;

struct TriangleVisitedInfoDM {

	Triangle *triangle;
	TriangleInfo *triangleInfo;
	bool visited;

	//disjoint mesh index
	short int disjointMeshIndex;

};

struct NodeVisitedInfo {

	ListVisitedInfoIndex listIndexes;
	bool visited;

};

//used if boost::unordered_map
struct PointHash {
	size_t operator()(const Point& p) const {

		//boost::hash<int> hasher;
		//size_t valHash = hasher(p.id()); //is not possible use id because it is different for each point, also same points

		boost::hash<double> hasher;
		//TODO: improve hash algorith
		//size_t valHash = hasher(CGAL::to_double( p.pp.x() )); //non e' una hash esatta, ogni volta potrebbe essere un diverso risultato
		//size_t valHash = hasher(CGAL::to_double( p.x()+ p.y() + p.z() )); //non e' una hash esatta, ogni volta potrebbe essere un diverso risultato
		size_t valHash = hasher(0.0);
		return valHash;
	}
};

//used if boost::unordered_map
struct PointEqual {
  bool operator()(const Point& c1, const Point& c2) const {
	//std::cout << "EQ" << std::endl;
    return c1 == c2;
  }
};

//typedef boost::unordered_map<Point, NodeVisitedInfo, PointHash, PointEqual> MapPointVisitedInfo;
typedef std::map<Point, NodeVisitedInfo> MapPointVisitedInfo;
typedef std::pair<Point, NodeVisitedInfo> PairPointVisitedInfo;

//For Box Collision
typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TriangleVisitedInfoDM*> BoxInt;

//Stuff for Join set

struct IntUnorderedPair {

	int first;
	int second;

};

//Hash using the Cantor pairing function
struct IntUnorderedPairHash {
	size_t operator()(const IntUnorderedPair& p) const {

		int a = p.first;
		int b = p.second;
		if (p.first > p.second)
		{
			 a = p.second;
			 b = p.first;
		}
		boost::hash<int> hasher;
		size_t valHash = hasher((a+b)*(a+b+1)/2 +b);

		return valHash;
	}
};

struct IntUnorderedPairEqual {
  bool operator()(const IntUnorderedPair& c1, const IntUnorderedPair& c2) const {
	//std::cout << "EQ" << std::endl;
    return ((c1.first == c2.first) && (c1.second == c2.second)) || ((c1.first == c2.second) && (c1.second == c2.first));
  }
};

typedef boost::unordered_set<IntUnorderedPair, IntUnorderedPairHash, IntUnorderedPairEqual> BoostSetIntUnorderedPair;
boost::thread_specific_ptr<BoostSetIntUnorderedPair> threadLocalIntUnorderedPair;
//TODO: use mutex if the CGAL function doesn't support Threads

#define DISJOINTMESH_TRIANGLE_SCALE_VALUE 1.1
//Scale triangle, used for improve do_intersect results: scale the triangle on its plane, centred in its barycenter
inline Triangle triangleScale(Triangle t)
{
	Plane p = t.supporting_plane();
	KPoint2 a = p.to_2d(t.vertex(0));
	KPoint2 b = p.to_2d(t.vertex(1));
	KPoint2 c = p.to_2d(t.vertex(2));
	KPoint2 baryc((a.x() + b.x() +c.x())/3.0, (a.y() + b.y() +c.y())/3.0);
	CGAL::Aff_transformation_2<Kernel> translate1(CGAL::TRANSLATION, CGAL::Vector_2<Kernel>(-baryc.x(), -baryc.y()));
	CGAL::Aff_transformation_2<Kernel> translate2(CGAL::TRANSLATION, CGAL::Vector_2<Kernel>(baryc.x(), baryc.y()));
	CGAL::Aff_transformation_2<Kernel> scale(CGAL::SCALING, DISJOINTMESH_TRIANGLE_SCALE_VALUE);
	CGAL::Aff_transformation_2<Kernel> transform = translate2 * (scale * translate1);
	KPoint2 an = transform(a);
	KPoint2 bn = transform(b);
	KPoint2 cn = transform(c);
	Point an3 = p.to_3d(an);
	Point bn3 = p.to_3d(bn);
	Point cn3 = p.to_3d(cn);
	Triangle tn(an3, bn3, cn3);
	return tn;
}

//Thread safe using boost::thread_specific_ptr for global variable
void reportIntersectionCallback(const BoxInt& b1, const BoxInt& b2) {

	TriangleVisitedInfoDM *m1 = b1.handle();
	TriangleVisitedInfoDM *m2 = b2.handle();

	if ((!m1->visited) || (!m2->visited))
	{
		std::cout << "ERROR: NOT TriangleVisitedInfoDM Visited!!!" << std::endl; //Error: this should not be: not visited triangle!
	}

	if (m1->disjointMeshIndex != m2->disjointMeshIndex)
	{
		IntUnorderedPair iup;
		iup.first = m1->disjointMeshIndex;
		iup.second = m2->disjointMeshIndex;

		BoostSetIntUnorderedPair::iterator foundIntUnorderedPair = threadLocalIntUnorderedPair.get()->find(iup);
		if (foundIntUnorderedPair == threadLocalIntUnorderedPair.get()->end())
		{
			//std::cout << m1->disjointMeshIndex << " - " << m2->disjointMeshIndex << std::endl;
			if ( (! m1->triangle->is_degenerate()) && (! m2->triangle->is_degenerate()) )
			{
				//if (CGAL::do_intersect( *(m1->triangle), *(m2->triangle) ) )
				if (CGAL::do_intersect( triangleScale(*(m1->triangle)), triangleScale(*(m2->triangle)) ) )
				{
					//std::cout << " INTERSECT!! " << std::endl;
					threadLocalIntUnorderedPair.get()->insert(iup);
				}
			}
		}
	}

}

std::list<MeshData> disjointNonContiguousMeshes(MeshData &meshData) {

	std::cout << "Executing disjointNonContiguousMeshes algorithm..." << std::endl;

	std::vector<MeshData> resultDisjointMeshes;
	MapPointVisitedInfo mapPointVisitedInfo;
	std::vector<TriangleVisitedInfoDM> vectorTriangleVisitedInfo;

	#ifdef DISJOINTMESH_LOG
	CGAL::Timer timer1;
	timer1.start();
	#endif

	//STEP 1: Popolo mapPointVisitedInfo and vectorTriangleVisitedInfo
	unsigned int vectorIndex = 0;

	std::list<Triangle>::iterator triangleIter;
	std::list<TriangleInfo>::iterator triangleInfoIter;
	for(triangleIter = meshData.first.begin(), triangleInfoIter = meshData.second.begin();
				triangleIter != meshData.first.end();
				++triangleIter, ++triangleInfoIter)
	{
		//Triangle t = *triangleIter;
		Triangle *tp = &(*triangleIter); //Use the pointer, it should not change into the container, since there aren't new added elements
		TriangleInfo *tip = &(*triangleInfoIter); //Use the pointer, it should not change into the container, since there aren't new added elements

		TriangleVisitedInfoDM tvi;
		//tvi.triangle = &t;
		tvi.triangle = tp;
		tvi.triangleInfo = tip;
		tvi.visited = false;
		tvi.disjointMeshIndex = -1; //TODO: patch per triangoli scollegati??? : tvi.disjointMeshIndex = -vectorIndex;

		vectorTriangleVisitedInfo.push_back(tvi);

		#ifdef DISJOINTMESH_LOG
		if ((vectorIndex % 1000) == 0)
		{
			//std::cout << " ::vectorIndex:" << vectorIndex << " load: " <<  mapPointVisitedInfo.load_factor() << " buckcount:" << mapPointVisitedInfo.bucket_count() << std::endl;
			std::cout << " ::vectorIndex:" << vectorIndex << std::endl;
		}
		#endif

		for (unsigned int i = 0;  i < 3; ++i)
		{
			Point v = (*tp).vertex(i);

			MapPointVisitedInfo::iterator foundPair = mapPointVisitedInfo.find(v);
			if (foundPair == mapPointVisitedInfo.end())
			{
				//Not found
				NodeVisitedInfo pvi;
				pvi.listIndexes.push_back(vectorIndex);
				pvi.visited = false;
				mapPointVisitedInfo.insert(PairPointVisitedInfo(v, pvi));
				//mapPointVisitedInfo[v] = pvi;
			}
			else
			{
				//Found
				ListVisitedInfoIndex *listTriangleVisitedInfoIndex = &((foundPair->second).listIndexes);
				listTriangleVisitedInfoIndex->push_back(vectorIndex);
			}

		}

		vectorIndex++;
	}

	#ifdef DISJOINTMESH_LOG
	//timer1.stop();
	std::cout << "timer1: " << timer1.time() << std::endl;
	std::cout << "mapPointVisitedInfo size:" << mapPointVisitedInfo.size() << std::endl;
	std::cout << "vectorTriangleVisitedInfo size:" << vectorTriangleVisitedInfo.size() << std::endl;
	//timer1.reset();
	//timer1.start();
	#endif

	//STEP 2: Disjoint break non continuos meshes algorithm
	short int disjointMeshIndex = 0; // counter of the disjoint mesh
	MapPointVisitedInfo::iterator mapPointVisitedInfoIter;
	for(mapPointVisitedInfoIter = mapPointVisitedInfo.begin(); mapPointVisitedInfoIter != mapPointVisitedInfo.end(); ++mapPointVisitedInfoIter)
	{
		//Cerco il primo punto non visitato e lo marco come visitato, e lancio la visita non ricorsive su di esso
		Point vStart = (*mapPointVisitedInfoIter).first;
		NodeVisitedInfo *pviStart = &((*mapPointVisitedInfoIter).second);

		if (!pviStart->visited)
		{
			//Mark point visited
			pviStart->visited = true;

			std::list<Point> pointsToVisit;//lista per la visita non ricorsiva
			pointsToVisit.push_back(vStart);

			//List to add the triangles for the disjoint mesh
			TrianglesList disjointTriangles;	     //TODO: should be an unordered_set<Triangle>
			TrianglesInfoList disjointTrianglesInfo; //TODO: should be an unordered_set<TriangleInfo>

			//Visit all triangles in pointsToVisit while is not empty
			while (!pointsToVisit.empty())
			{
				Point v = pointsToVisit.front();
				pointsToVisit.pop_front();

				//Aggiungo a pointsToVisit i punti connessi con p non ancora visitati:
				// ciclo i triangoli adiacenti a p, per ogni triangolo se non gia' visitato: lo marco visitato,
				//  lo aggiungo a disjointMesh e ciclo i vertici non visitati e li aggiungo a pointsToVisit, marcandoli visitati
				NodeVisitedInfo *pvi = &mapPointVisitedInfo[v];
				ListVisitedInfoIndex::iterator pviIndexIterator;
				for(pviIndexIterator = pvi->listIndexes.begin(); pviIndexIterator != pvi->listIndexes.end(); ++pviIndexIterator)
				{
					unsigned int triangleIndex = *pviIndexIterator;
					TriangleVisitedInfoDM *tvi = &(vectorTriangleVisitedInfo.at(triangleIndex));
					if (!tvi->visited)
					{
						//Mark triangle visited
						tvi->visited = true;

						//Add triangle to the mesh List
						disjointTriangles.push_back(*(tvi->triangle));
						disjointTrianglesInfo.push_back(*(tvi->triangleInfo));
						//std::cout << "*";

						//Set disjointMeshIndex
						tvi->disjointMeshIndex = disjointMeshIndex;

						for (unsigned int i = 0;  i < 3; ++i)
						{
							Point vAdjacent = (*(tvi->triangle)).vertex(i);
							NodeVisitedInfo *pviAdjacent = &mapPointVisitedInfo[vAdjacent];
							if (!pviAdjacent->visited)
							{
								pviAdjacent->visited = true;
								pointsToVisit.push_back(vAdjacent);
							}
						}
					}
				}

				//Use p for visit: usual for postvisit
				//......
			}

			if (disjointTriangles.size() != 0)
			{
				//std::cout << "Adding a resultDisjointMeshes mesh..." << std::endl;
				resultDisjointMeshes.push_back(MeshData(disjointTriangles, disjointTrianglesInfo));

				//Increment the disjoint mesh index
				disjointMeshIndex++;
			}
		}
	}

	#ifdef DISJOINTMESH_LOG
	std::cout << "resultDisjointMeshes size: " << resultDisjointMeshes.size() << std::endl;

	//timer1.stop();
	std::cout << "timer1: " << timer1.time() << std::endl;
	#endif

	//STEP 3: Boxes intersections computation
	std::vector<BoxInt> boxes;

	//create the boxes
	std::vector<TriangleVisitedInfoDM>::iterator vectorTriangleVisitedInfoIter;
	for(vectorTriangleVisitedInfoIter = vectorTriangleVisitedInfo.begin();
			vectorTriangleVisitedInfoIter != vectorTriangleVisitedInfo.end(); ++vectorTriangleVisitedInfoIter)
	{
		TriangleVisitedInfoDM *tvi = &(*vectorTriangleVisitedInfoIter);  //Use the pointer, it should not change into the container, since there aren't new added elements
		/*
		CGAL::Bbox_3 tBBox = tvi->triangle->bbox();
		double delta = 0.1;
		CGAL::Bbox_3 tBBoxGrow(tBBox.xmin() - delta, tBBox.ymin() - delta, tBBox.zmin() - delta,
				tBBox.xmax() + delta, tBBox.ymax() + delta, tBBox.zmax() + delta);
		boxes.push_back( BoxInt( tBBoxGrow, tvi ));
		*/
		boxes.push_back( BoxInt( tvi->triangle->bbox(), tvi ));
	}

	//Init BoostSetIntUnorderedPair in the thread
	threadLocalIntUnorderedPair.reset(new BoostSetIntUnorderedPair);
	threadLocalIntUnorderedPair.get()->clear(); //init redundant

	//Do intersection
	CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), reportIntersectionCallback);

	#ifdef DISJOINTMESH_LOG
	std::cout << "threadLocalIntUnorderedPair size: " << threadLocalIntUnorderedPair.get()->size() << std::endl;

	//timer1.stop();
	std::cout << "timer1: " << timer1.time() << std::endl;
	#endif


	//STEP 4: join adjacent meshes in resultDisjointMeshes
	std::list<MeshData> resultRejointMeshes;

	//build structures for graph visit
	//init nodeAdjacentInfo
	std::vector<NodeVisitedInfo> nodeAdjacentInfo;
	for (unsigned int nodeIndex = 0;  nodeIndex < resultDisjointMeshes.size(); ++nodeIndex)
	{
		NodeVisitedInfo nvi;
		nvi.visited = false;
		nodeAdjacentInfo.push_back(nvi);
	}

	//populate nodeAdjacentInfo, cycle for adjacent pairs
	BoostSetIntUnorderedPair::iterator setIntUnorderedPairIter;
	for(setIntUnorderedPairIter = threadLocalIntUnorderedPair.get()->begin(); setIntUnorderedPairIter != threadLocalIntUnorderedPair.get()->end(); ++setIntUnorderedPairIter)
	{
		int first = (*setIntUnorderedPairIter).first;
		int second = (*setIntUnorderedPairIter).second;
		#ifdef DISJOINTMESH_LOG
		std::cout << "ADJ: (" << first << ", " << second << ") " << std::endl;
		#endif

		nodeAdjacentInfo[first].listIndexes.push_back(second);
		nodeAdjacentInfo[second].listIndexes.push_back(first);
	}

	#ifdef DISJOINTMESH_LOG
	//Dump Adjacency matrix for debug
	std::cout << "Adjacency Matrix:" << std::endl;
	for (unsigned int nodeIndex = 0;  nodeIndex < nodeAdjacentInfo.size(); ++nodeIndex)
	{
		std::cout << nodeIndex << ":: ";
		NodeVisitedInfo nvi = nodeAdjacentInfo.at(nodeIndex);
		ListVisitedInfoIndex::iterator nviIndexIterator;
		for(nviIndexIterator = nvi.listIndexes.begin(); nviIndexIterator != nvi.listIndexes.end(); ++nviIndexIterator)
		{
			unsigned int connectedIndex = *nviIndexIterator;
			std::cout << connectedIndex << "  ";
		}
		std::cout << std::endl;
	}
	#endif

	//Visit graph
	for (unsigned int nodeIndex = 0;  nodeIndex < nodeAdjacentInfo.size(); ++nodeIndex)
	{
		NodeVisitedInfo *nvi = &nodeAdjacentInfo.at(nodeIndex);
		//Cerco il primo nodo non visitato e lo marco come visitato, e lancio la visita non ricorsive su di esso
		if (!nvi->visited)
		{
			//Mark node visited
			nvi->visited = true;

			ListVisitedInfoIndex nodesToVisit; //lista per la visita non ricorsiva
			nodesToVisit.push_back(nodeIndex);

			//List to add the triangles for the rejoint mesh
			TrianglesList rejointTriangles;	     //TODO: should be an unordered_set<Triangle>
			TrianglesInfoList rejointTrianglesInfo; //TODO: should be an unordered_set<TriangleInfo>

			//Visit all nodes in nodesToVisit while is not empty
			while (!nodesToVisit.empty())
			{
				int nodeToVisitIndex = nodesToVisit.front();
				nodesToVisit.pop_front();

				//Aggiungo a nodesToVisit i nodi connessi con nodeToVisitIndex non ancora visitati e li marco visitati:
				NodeVisitedInfo *nviToVisit = &nodeAdjacentInfo.at(nodeToVisitIndex);
				ListVisitedInfoIndex::iterator nviIndexIterator;
				for(nviIndexIterator = nviToVisit->listIndexes.begin(); nviIndexIterator != nviToVisit->listIndexes.end(); ++nviIndexIterator)
				{
					unsigned int connectedIndex = *nviIndexIterator;
					NodeVisitedInfo *nviConnected = &nodeAdjacentInfo.at(connectedIndex);
					if (!nviConnected->visited)
					{
						//Mark node visited
						nviConnected->visited = true;
						nodesToVisit.push_back(connectedIndex);
					}
				}

				//Rejoint mesh at index nodeToVisitIndex
				#ifdef DISJOINTMESH_LOG
				std::cout << "----> node : " << nodeToVisitIndex << std::endl;
				#endif
				rejointTriangles.insert(rejointTriangles.end(),
						resultDisjointMeshes.at(nodeToVisitIndex).first.begin(),
						resultDisjointMeshes.at(nodeToVisitIndex).first.end());
				rejointTrianglesInfo.insert(rejointTrianglesInfo.end(),
						resultDisjointMeshes.at(nodeToVisitIndex).second.begin(),
						resultDisjointMeshes.at(nodeToVisitIndex).second.end());
			}

			if (rejointTriangles.size() != 0)
			{
				#ifdef DISJOINTMESH_LOG
				std::cout << "------------> Adding a resultRejointMeshes mesh..." << std::endl;
				#endif
				resultRejointMeshes.push_back(MeshData(rejointTriangles, rejointTrianglesInfo));
			}

		}
	}

	//Release the pointer to BoostSetIntUnorderedPair
	delete threadLocalIntUnorderedPair.get();
	threadLocalIntUnorderedPair.release();

	#ifdef DISJOINTMESH_LOG
	timer1.stop();
	std::cout << "timer1: " << timer1.time() << std::endl;
	#endif

	//TODO: check se le mesh sono innestate (usare aabb innestati)

	std::cout << "end disjointNonContiguousMeshes algorithm size:" << resultRejointMeshes.size() << std::endl;

	return resultRejointMeshes;
}


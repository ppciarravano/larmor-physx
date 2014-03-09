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

//#define CGAL_SURFACE_SIMPLIFICATION_ENABLE_LT_TRACE 0
//#define CGAL_SURFACE_SIMPLIFICATION_ENABLE_TRACE 0

#include "DelaunayVoronoi.h"

//log some messages
#define MESHSIMPLIFICATION_LOG

#include <iostream>
#include <limits>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <boost/unordered_map.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/number_utils.h>
#include <CGAL/squared_distance_3.h>
#include <CGAL/Timer.h>
#include <CGAL/exceptions.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Polyhedron_3.h>

//Includes for mesh simplification
// Adaptor for Polyhedron_3
#include <CGAL/Surface_mesh_simplification/HalfedgeGraph_Polyhedron_3.h>
// Simplification function
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
// Stop-condition policy
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
// Visitor base
#include <CGAL/Surface_mesh_simplification/Edge_collapse_visitor_base.h>
// Extended polyhedron items which include an id() field
//#include <CGAL/Polyhedron_items_with_id_3.h>
// Stop-condition policy
//#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
// Non-default cost and placement policies
//#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_and_length.h>
namespace SMS = CGAL::Surface_mesh_simplification;

typedef CGAL::Simple_cartesian<double> KernelForPolyhedron;
//typedef CGAL::Exact_predicates_inexact_constructions_kernel KernelForPolyhedron;
typedef KernelForPolyhedron::FT FTKK;
typedef KernelForPolyhedron::Point_3 PointKK;
typedef CGAL::Polyhedron_3<KernelForPolyhedron>         Polyhedron;
typedef Polyhedron::HalfedgeDS           HalfedgeDS;

//typedef for mesh simplification
//typedef CGAL::Polyhedron_3<KernelForPolyhedron, CGAL::Polyhedron_items_with_id_3> Polyhedron;
//typedef Polyhedron::HalfedgeDS      HalfedgeDS;

typedef std::map<PointCGAL, int> MapPointId;
typedef std::pair<PointCGAL, int> PairPointId;

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

struct EdgeVisitedInfo {

	bool visited;
	IntUnorderedPair edgeVertexes;
	std::vector<unsigned int> triangleVisitedInfoIndexes;

};

struct TriangleVisitedInfoMS {

	Triangle *triangle;
	bool visited;

	std::vector<unsigned int> vertexIndexes;
	std::vector<unsigned int> edgeVisitedInfoIndexes;

};

typedef boost::unordered_map<IntUnorderedPair, int, IntUnorderedPairHash, IntUnorderedPairEqual> MapUnorderedPairId;
typedef std::pair<IntUnorderedPair, int> PairIntUnorderedPairId;


//Build_triangle_mesh_with_id for build the Polyhedron
template <class HDS>
class Build_triangle_mesh_coherent_surface : public CGAL::Modifier_base<HDS> {

	TrianglesList *trianglesList;

	MapPointId mapPointId;
	std::vector<PointCGAL> vertexPoints;

	std::vector<TriangleVisitedInfoMS> trianglesVisited;

	MapUnorderedPairId mapEdgeId;
	std::vector<EdgeVisitedInfo> edgesVisited;

	public:
	Build_triangle_mesh_coherent_surface(TrianglesList& triangles) {
		trianglesList = &triangles;
	}

    void operator()( HDS& hds) {

    	CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, false);
		//B.begin_surface( 3*trianglesList->size(), trianglesList->size(), 0, CGAL::Polyhedron_incremental_builder_3<HDS>::RELATIVE_INDEXING); //ABSOLUTE_INDEXING
		B.begin_surface( 3*trianglesList->size(), trianglesList->size()); //ABSOLUTE_INDEXING

		// Init data structures and add points to B
		int pointIndex = 0;
		int triangleIndex = 0;
		int edgeIndex = 0;
		std::list<Triangle>::iterator triangleIter;
		for(triangleIter = trianglesList->begin(); triangleIter != trianglesList->end(); ++triangleIter)
		{
			Triangle t = *triangleIter;

			if (t.is_degenerate())
			{
				continue;
			}

			TriangleVisitedInfoMS tvi;
			tvi.triangle = &(*triangleIter); //Use the pointer, it should not change into the container, since there aren't new added elements
			tvi.visited = false;

			// loop for vertexes
			for (int i = 0;  i < 3; ++i)
			{
				PointCGAL v = t.vertex(i);

				MapPointId::iterator foundPair = mapPointId.find(v);
				if (foundPair == mapPointId.end())
				{
					// PointCGAL not found
					Polyhedron::Vertex_handle vh = B.add_vertex( PointKK(CGAL::to_double(v.x()), CGAL::to_double(v.y()), CGAL::to_double(v.z())) );

					mapPointId.insert(PairPointId(v, pointIndex));
					vertexPoints.push_back(v);
					tvi.vertexIndexes.push_back(pointIndex);
					pointIndex++;
				}
				else
				{
					tvi.vertexIndexes.push_back(foundPair->second);
				}
			}

			// loop for edges
			for (int i = 0;  i < 3; ++i)
			{
				int firstEdgeIndex = i;
				int secondEdgeIndex = (i<2) ? (i+1) : 0;
				IntUnorderedPair iup;
				iup.first = tvi.vertexIndexes.at(firstEdgeIndex);
				iup.second = tvi.vertexIndexes.at(secondEdgeIndex);

				MapUnorderedPairId::iterator foundPair = mapEdgeId.find(iup);
				if (foundPair == mapEdgeId.end())
				{
					// Edge not found
					EdgeVisitedInfo evi;
					evi.edgeVertexes = iup;
					evi.visited = false;
					evi.triangleVisitedInfoIndexes.push_back(triangleIndex);

					mapEdgeId.insert(PairIntUnorderedPairId(iup, edgeIndex));
					edgesVisited.push_back(evi);

					tvi.edgeVisitedInfoIndexes.push_back(edgeIndex);
					edgeIndex++;
				}
				else
				{
					EdgeVisitedInfo *evi = &(edgesVisited.at(foundPair->second));
					evi->triangleVisitedInfoIndexes.push_back(triangleIndex);
					tvi.edgeVisitedInfoIndexes.push_back(foundPair->second);
				}

			}

			trianglesVisited.push_back(tvi);
			triangleIndex++;
		}


		// build the surface adding the triangles
		std::queue<EdgeVisitedInfo*> fifoEdgesToVisit;
		for (unsigned int edgeIdx = 0; edgeIdx < edgesVisited.size(); edgeIdx++)
		{
			EdgeVisitedInfo *evip = &edgesVisited.at(edgeIdx);
			if (!(evip->visited))
			{
				//std::cout << "START";
				fifoEdgesToVisit.push(evip);
				while(!fifoEdgesToVisit.empty())
				{
					EdgeVisitedInfo *evitv = fifoEdgesToVisit.front();
					fifoEdgesToVisit.pop();
					//if (!(evitv->visited))
					{
						//std::cout << "*";
						evitv->visited = true;
						for (unsigned int triangleVecIdx = 0; triangleVecIdx < evitv->triangleVisitedInfoIndexes.size(); triangleVecIdx++)
						{
							TriangleVisitedInfoMS *tvitv =  &trianglesVisited.at(evitv->triangleVisitedInfoIndexes.at(triangleVecIdx));
							if (!(tvitv->visited))
							{
								// check if the triangle edges have more than 2 connected triangles
								// and add the connected non visited edges to fifoEdgesToVisit
								bool oneEdgeHasMoreThanTwoTriangles = false;
								for (unsigned int edgeVecIdx = 0; edgeVecIdx < tvitv->edgeVisitedInfoIndexes.size(); edgeVecIdx++)
								{
									EdgeVisitedInfo *eviTvitv = &edgesVisited.at(tvitv->edgeVisitedInfoIndexes.at(edgeVecIdx));
									if (!(eviTvitv->visited))
									{
										fifoEdgesToVisit.push(eviTvitv);
									}

									if (eviTvitv->triangleVisitedInfoIndexes.size() > 2)
									{
										oneEdgeHasMoreThanTwoTriangles = true;
									}
								}

								if (oneEdgeHasMoreThanTwoTriangles)
								{
									//std::cout << "2";
									// Add new points and the triangle with these new vertexes
									std::vector<unsigned int> newVertexIndexes;
									for (int vIdx = 0; vIdx < tvitv->vertexIndexes.size(); vIdx++)
									{
										PointCGAL vt = vertexPoints.at(tvitv->vertexIndexes.at(vIdx));
										B.add_vertex( PointKK(CGAL::to_double(vt.x()), CGAL::to_double(vt.y()), CGAL::to_double(vt.z())) );
										vertexPoints.push_back(vt);
										newVertexIndexes.push_back(pointIndex);
										pointIndex++;
									}
									if (B.test_facet(newVertexIndexes.begin(), newVertexIndexes.end()))
									{
										B.add_facet(newVertexIndexes.begin(), newVertexIndexes.end());
									}
									else
									{
										std::cout << "Build_triangle_mesh_coherent_surface: WARNING: can't add a triangle in polyhedron (1)!!!\n";
									}
								}
								else
								{
									//add triangle tvitv to Polyhedron surface
									bool testFacetVerseA = B.test_facet(tvitv->vertexIndexes.begin(), tvitv->vertexIndexes.end());
									if (testFacetVerseA)
									{
										//std::cout << "A";
										B.add_facet(tvitv->vertexIndexes.begin(), tvitv->vertexIndexes.end());
									}
									else
									{

										bool testFacetVerseB = B.test_facet(tvitv->vertexIndexes.rbegin(), tvitv->vertexIndexes.rend());
										if (testFacetVerseB)
										{
											//std::cout << "B";
											B.add_facet(tvitv->vertexIndexes.rbegin(), tvitv->vertexIndexes.rend());
										}
										else
										{
											//std::cout << "C"; // Same block of 2
											// Add new points and the triangle with these new vertexes
											std::vector<unsigned int> newVertexIndexes;
											for (int vIdx = 0; vIdx < tvitv->vertexIndexes.size(); vIdx++)
											{
												PointCGAL vt = vertexPoints.at(tvitv->vertexIndexes.at(vIdx));
												B.add_vertex( PointKK(CGAL::to_double(vt.x()), CGAL::to_double(vt.y()), CGAL::to_double(vt.z())) );
												vertexPoints.push_back(vt);
												newVertexIndexes.push_back(pointIndex);
												pointIndex++;
											}
											if (B.test_facet(newVertexIndexes.begin(), newVertexIndexes.end()))
											{
												B.add_facet(newVertexIndexes.begin(), newVertexIndexes.end());
											}
											else
											{
												std::cout << "Build_triangle_mesh_coherent_surface: WARNING: can't add a triangle in polyhedron (2)!!!\n";
											}
										}
									}
								}

								tvitv->visited = true;
							}
						}
					}
				}
			}
		}


		if ( B.check_unconnected_vertices() )
		{
			//std::cout << "Remove_unconnected_vertices" << std::endl;
			B.remove_unconnected_vertices();
		}

		B.end_surface();
    }

};


TrianglesList meshSimplification(TrianglesList &triangles, int stopPredicate) {

	#ifdef MESHSIMPLIFICATION_LOG
	CGAL::Timer timer;
	timer.start();
	#endif

	TrianglesList result;

	try
	{
		Polyhedron P;

		#ifdef MESHSIMPLIFICATION_LOG
		std::cout << "Start Building Polyhedron surface... " << std::endl;
		#endif

		Build_triangle_mesh_coherent_surface<HalfedgeDS> triangle(triangles);
		P.delegate(triangle);
		P.normalize_border();

		#ifdef MESHSIMPLIFICATION_LOG
		std::cout << "Completed Building Polyhedron surface:" << std::endl;
		std::cout << "Polyhedron is_pure_triangle: " << P.is_pure_triangle() << std::endl;
		std::cout << "Polyhedron is_closed: " << P.is_closed() << std::endl;
		std::cout << "Polyhedron is_pure_bivalent : " << P.is_pure_bivalent () << std::endl;
		std::cout << "Polyhedron is_pure_trivalent: " << P.is_pure_trivalent() << std::endl;
		std::cout << "Polyhedron is_valid 0: " << P.is_valid(false, 0) << std::endl;
		std::cout << "Polyhedron is_valid 1: " << P.is_valid(false, 1) << std::endl;
		std::cout << "Polyhedron is_valid 2: " << P.is_valid(false, 2) << std::endl;
		std::cout << "Polyhedron is_valid 3: " << P.is_valid(false, 3) << std::endl;
		std::cout << "Polyhedron is_valid 4: " << P.is_valid(false, 4) << std::endl;
		std::cout << "Polyhedron normalized_border_is_valid : " << P.normalized_border_is_valid(false) << std::endl;
		#endif

		#ifdef MESHSIMPLIFICATION_LOG
		std::cout << "Start edge_collapse... " << std::endl;
		#endif

		SMS::Count_stop_predicate<Polyhedron> stop(stopPredicate);

		int removedEdges = SMS::edge_collapse(P, stop,
				CGAL::vertex_index_map(boost::get(CGAL::vertex_external_index, P)).edge_index_map(boost::get(CGAL::edge_external_index ,P))
		);

		#ifdef MESHSIMPLIFICATION_LOG
		std::cout << "Completed edge_collapse:" << std::endl;
		std::cout << "Finished with: " << removedEdges << " edges removed and "  << (P.size_of_halfedges()/2) << " final edges." << std::endl;
		#endif

		//Build output result
		for ( Polyhedron::Facet_iterator fit( P.facets_begin() ), fend( P.facets_end() ); fit != fend; ++fit )
		{
			if ( fit->is_triangle() )
			{
				PointCGAL verts[3];
				int tick = 0;

				Polyhedron::Halfedge_around_facet_circulator hit( fit->facet_begin() ), hend( hit );
				do
				{
					if ( tick < 3 )
					{
						verts[tick++] = PointCGAL( hit->vertex()->point().x(), hit->vertex()->point().y(), hit->vertex()->point().z() );
					}
					else
					{
						std::cout << "meshSimplification: We've got facets with more than 3 vertices even though the facet reported to be triangular..." << std::endl;
					}

				} while( ++hit != hend );

				result.push_back( Triangle(verts[0], verts[1], verts[2]) );
			}
			else
			{
				std::cout << "meshSimplification: Skipping non-triangular facet" << std::endl;
			}

		}

	}
	catch (CGAL::Assertion_exception e)
	{
		std::cout << "ERROR: meshSimplification CGAL::Assertion_exception" << e.message() << std::endl;
	}

	#ifdef MESHSIMPLIFICATION_LOG
	timer.stop();
	std::cout << "meshSimplification result with: " << result.size() << " triangles." << std::endl;
	std::cout << "Total meshSimplification time: " << timer.time() << std::endl;
	#endif

	return result;
}


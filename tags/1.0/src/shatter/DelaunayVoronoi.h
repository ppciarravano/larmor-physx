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

#ifndef DELAUNAYVORONOI_H_
#define DELAUNAYVORONOI_H_

#include <iostream>
#include <fstream>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Random.h>
#include <CGAL/point_generators_3.h>
#include <algorithm>
#include <CGAL/algorithm.h>
#include <CGAL/function_objects.h>
#include <CGAL/barycenter.h>
#include <CGAL/Aff_transformation_3.h>
#include <CGAL/bounding_box.h>
#include <CGAL/centroid.h>
//#include <CGAL/barycenter.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
//#include <boost/serialization/binary_object.hpp>

#include "Custom.h"
#include "CutterMesher.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel KD;
typedef CGAL::Delaunay_triangulation_3<KD, CGAL::Fast_location> Delaunay;
typedef Delaunay::Point DPoint_3;
typedef Delaunay::Edge            DEdge;
typedef Delaunay::Segment       DSegment_3;
typedef Delaunay::Vertex_handle  DVertex_handle;

typedef KD::Point_3                              KDPoint;
typedef KD::Point_2                              KDPoint_2;
typedef KD::Vector_3                             KDVector;
typedef CGAL::Creator_uniform_3<double,KDPoint>  Pt_creator;
typedef CGAL::Aff_transformation_3<KD>           KDTransformation;
typedef KD::Triangle_3                           KDTriangle;

typedef std::pair<MeshData,Point> TranslatedMeshData;
//typedef std::list<TranslatedMeshData> TranslatedMeshDataList;

//Main voronoiShatter Function
std::list<MeshData> voronoiShatter(MeshData &meshData, std::list<KDPoint> points);

//Util Functions
std::list<KDPoint> randomPointsInSphere(int numPoints, double radius);
std::list<KDPoint> randomPointsInSphereGaussianDistribution(int numPoints, double radius, double width);
std::list<KDPoint> randomPointsInConcaveSphere(int numPoints, double innerRadius, double outerRadius);
std::list<KDPoint> randomPointsOnSphereSurface(int numPoints, double radius);
std::list<KDPoint> randomPointsInCube(int numPoints, double radius);
std::list<KDPoint> randomPointsInBox(int numPoints, KDPoint min, KDPoint max);
std::pair<Point,Point> getMeshBoundingBox(MeshData &meshData);
KDPoint converPointExactToInexact(Point point);
Point converPointInexactToExact(KDPoint point);
Point getMeshBarycenter(MeshData &meshData);
MeshData translateMesh(MeshData &meshData, Point point);
MeshData scaleMesh(MeshData &meshData, double scaleFactor);
TrianglesList scaleAndCenterMesh(TrianglesList &meshTriangles, double scaleFactor);
std::list<TranslatedMeshData> centerMeshesInBarycenter(std::list<MeshData> &listMeshData);
std::list<KDPoint> translatePoints(std::list<KDPoint> &listPoints, KDPoint vectorPoint);

//Complete Voronoi Shatter distribution functions
std::list<MeshData> voronoiShatter_uniformDistributionPoints(MeshData &meshData, int numPoints, bool doDisjointMesh);
std::list<MeshData> voronoiShatter_uniformDistributionPoints(TrianglesList &meshTriangles, int numPoints, bool doDisjointMesh);
std::list<MeshData> voronoiShatter_sphereDistributionOnPoint(MeshData &meshData, int numPoints, KDPoint targetPoint, double radius, bool doDisjointMesh);
std::list<MeshData> voronoiShatter_sphereDistributionOnPoint(TrianglesList &meshTriangles, int numPoints, KDPoint targetPoint, double radius, bool doDisjointMesh);

//File Input and Output functions
#define DELAUNAYVORONOI_BINARY_FILE_TYPE 0
#define DELAUNAYVORONOI_TEXT_FILE_TYPE 1
//template <class SerializeType> void saveTranslatedMeshDataList(SerializeType &transMesh, const char * filename, const int fileType);
//template <class SerializeType> SerializeType loadTranslatedMeshDataList(const char * filename, const int fileType);
void saveTranslatedMeshDataList(std::list<TranslatedMeshData> &transMesh, const char * filename, const int fileType);
std::list<TranslatedMeshData> loadTranslatedMeshDataList(const char * filename, const int fileType);

//Functions definited in VolumeMesh.cpp
// internalVolumeAndInertiaMesh is able to calculate correctly a volume and inertia tensor of an empty holed mesh; calculateMassCenterInertia doesn't
double minimumEdgeLength(TrianglesList &triangles); //use to find a valid facetDistance (facetDistance can be minimum * 10);
double minimumBoundingDimension(TrianglesList &triangles);
std::vector<double> internalVolumeAndInertiaMesh(TrianglesList &triangles, double facetDistance); //Use facetDistance = 0.01
std::vector<double> internalVolumeAndInertiaMeshConvex(TrianglesList &triangles, Point meshCenter);

//Functions definited in MeshMassProperties.cpp: doesn't work fine for holed meshes (and also for convex mesh too)
void calculateMassCenterInertia(TrianglesList &triangles, double &massReturn, Point &cmReturn, Vector &inertiaReturn);

//Functions definited in MeshSimplification.cpp
TrianglesList meshSimplification(TrianglesList &triangles, int stopPredicate);

//Functions definited in MeshCheck.cpp
TrianglesList meshRemoveIntersectedTriangles(TrianglesList &triangles);

//Function for call disjoint on std::list<MeshData>
std::list<MeshData> disjointNonContiguousListMeshes(std::list<MeshData> &listMeshData);
//Function definited in DisjointMesh.cpp
std::list<MeshData> disjointNonContiguousMeshes(MeshData &meshData);

//Function to rejointMeshDistance algorithm; it uses the functions in RejointMeshes.cpp
std::list<MeshData> rejointMeshesInDistance(std::list<MeshData> &listMeshData, KDPoint targetPoint, double distance, double isExternJoin);
//Funtion for rejoint Meshes, definited in RejointMeshes.cpp
MeshData rejointMeshes(std::list<MeshData> &listMeshData);
MeshData simpleRejointMeshes(std::list<MeshData> &listMeshData);


//Boost Serialization extend of text_oarchive and binary_oarchive with points mapping
namespace boost {
	namespace archive {

		struct TripleDouble {
			double d1;
			double d2;
			double d3;
		};

		class PointsMapperArchive {

			std::map<Point, TripleDouble> mapPointsDoubleValues;

			public:

				TripleDouble getMappedPoint(Point p);
		};

		class text_oarchive_pointsmap: public text_oarchive, public PointsMapperArchive {

			public:

			text_oarchive_pointsmap(std::ofstream &ofs);

		};

		class binary_oarchive_pointsmap: public binary_oarchive, public PointsMapperArchive {

			public:

			binary_oarchive_pointsmap(std::ofstream &ofs);
		};

	} // namespace archive
} // namespace boost

//Boost Serialization template for Point, Triangle and TriangleInfo
namespace boost {
	namespace serialization {

		//Point
		template<class Archive>
		void save(Archive & ar, const Point & t, unsigned int version)
		{
			//std::cout << "save P" << std::endl;
			/*
			//Use this block only with boost::archive::binary_oarchive or boost::archive::text_oarchive
			double x = CGAL::to_double(t.x());
			double y = CGAL::to_double(t.y());
			double z = CGAL::to_double(t.z());
			ar & x;
			ar & y;
			ar & z;
			*/
			//Use this block with boost::archive::text_oarchive_pointsmap or boost::archive::binary_oarchive_pointsmap
			boost::archive::PointsMapperArchive *toapm = dynamic_cast<boost::archive::PointsMapperArchive*>(&ar);
			if (toapm == 0)
			{
				double x = CGAL::to_double(t.x());
				double y = CGAL::to_double(t.y());
				double z = CGAL::to_double(t.z());
				ar & x;
				ar & y;
				ar & z;
			}
			else
			{
				//boost::archive::text_oarchive_pointsmap *toapm = static_cast<boost::archive::text_oarchive_pointsmap*>(&ar);
				//boost::archive::binary_oarchive_pointsmap *toapm = static_cast<boost::archive::binary_oarchive_pointsmap*>(&ar);
				boost::archive::TripleDouble td = toapm->getMappedPoint(t);
				ar & td.d1;
				ar & td.d2;
				ar & td.d3;
			}
		}

		template<class Archive>
		void load(Archive & ar, Point & t, unsigned int version)
		{
			//std::cout << "load P" << std::endl;
			double x, y, z;
			ar & x;
			ar & y;
			ar & z;
			Point newT(x, y, z);
			t = newT;
		}

		template<class Archive>
		inline void serialize(Archive & ar, Point & t, const unsigned int file_version)
		{
			//std::cout << "serialize P" << std::endl;
			split_free(ar, t, file_version);
		}
		//BOOST_SERIALIZATION_SPLIT_FREE(Point)

		//Triangle
		template<class Archive>
		void save(Archive & ar, const Triangle & t, unsigned int version)
		{
			//std::cout << "save T" << std::endl;
			Point t1 = t.vertex(0);
			Point t2 = t.vertex(1);
			Point t3 = t.vertex(2);
			ar & t1;
			ar & t2;
			ar & t3;
		}

		template<class Archive>
		void load(Archive & ar, Triangle & t, unsigned int version)
		{
			//std::cout << "load T" << std::endl;
			Point t1, t2, t3;
			ar & t1;
			ar & t2;
			ar & t3;
			Triangle newT(t1, t2, t3);
			t = newT;
		}

		template<class Archive>
		inline void serialize(Archive & ar, Triangle & t, const unsigned int file_version)
		{
			//std::cout << "serialize T" << std::endl;
			split_free(ar, t, file_version);
		}
		//BOOST_SERIALIZATION_SPLIT_FREE(Triangle)

		//TriangleInfo
 		template<class Archive>
		inline void serialize(Archive & ar, TriangleInfo & t, const unsigned int file_version)
		{
 			//std::cout << "serialize I" << std::endl;
 			ar & t.cutType;
		}

	} // namespace serialization
} // namespace boost



#endif /* DELAUNAYVORONOI_H_ */

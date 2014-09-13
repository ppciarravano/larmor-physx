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

//#define DELAUNAYVORONOI_USE_TBB
#ifdef DELAUNAYVORONOI_USE_TBB
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_do.h"
#include "tbb/concurrent_vector.h"
#include "tbb/mutex.h"
#endif


std::list<KDPoint> randomPointsInSphere(int numPoints, double radius)
{
	CGAL::Random_points_in_sphere_3<KDPoint, Pt_creator> sphereRnd(radius);
	std::list<KDPoint> randomPoints;
	CGAL::copy_n(sphereRnd, numPoints, std::back_inserter(randomPoints));
	return randomPoints;
}

std::list<KDPoint> randomPointsInSphereGaussianDistribution(int numPoints, double radius, double width)
{
	CGAL::Random_points_in_sphere_3<KDPoint, Pt_creator> sphereRnd(1.0);
	std::list<KDPoint> innerRandomPoints;
	CGAL::copy_n(sphereRnd, numPoints, std::back_inserter(innerRandomPoints));

	std::list<KDPoint> randomPoints;
	std::list<KDPoint>::iterator pointsIter;
	for(pointsIter = innerRandomPoints.begin(); pointsIter != innerRandomPoints.end(); ++pointsIter)
	{
		KDPoint point = *pointsIter;
		KDVector vPointR = KDVector(point.x(), point.y(), point.z());
		double r =  CGAL::sqrt(vPointR.squared_length());
		KDVector unitVector = vPointR / r;
		double gaussRadius = std::exp(-(width*r*r));
		KDTransformation scaleGauss(CGAL::SCALING, gaussRadius);
		KDTransformation scaleRadius(CGAL::SCALING, radius);
		KDPoint scaledPoint = scaleGauss(KDPoint(unitVector.x(), unitVector.y(), unitVector.z()));
		scaledPoint = scaleRadius(scaledPoint);
		randomPoints.push_back(scaledPoint);
	}
	return randomPoints;
}

std::list<KDPoint> randomPointsInConcaveSphere(int numPoints, double innerRadius, double outerRadius)
{
	CGAL::Random_points_in_sphere_3<KDPoint, Pt_creator> sphereRnd(outerRadius - innerRadius);
	std::list<KDPoint> innerRandomPoints;
	CGAL::copy_n(sphereRnd, numPoints, std::back_inserter(innerRandomPoints));

	std::list<KDPoint> randomPoints;
	std::list<KDPoint>::iterator pointsIter;
	for(pointsIter = innerRandomPoints.begin(); pointsIter != innerRandomPoints.end(); ++pointsIter)
	{
		KDPoint point = *pointsIter;
		KDVector vPoint1 = KDVector(point.x(), point.y(), point.z());
		KDVector vPoint1Unit = vPoint1 / CGAL::sqrt(vPoint1.squared_length());
		KDVector vPoint2 = vPoint1Unit * innerRadius;
		KDVector vPoint = vPoint1 + vPoint2;
		randomPoints.push_back(KDPoint(vPoint.x(), vPoint.y(), vPoint.z()));
	}
	return randomPoints;
}

std::list<KDPoint> randomPointsOnSphereSurface(int numPoints, double radius)
{
	/*
	//Old version using Random_points_in_sphere_3 and not Random_points_on_sphere_3
	CGAL::Random_points_in_sphere_3<KDPoint, Pt_creator> sphereRnd(1.0);
	std::list<KDPoint> innerRandomPoints;
	CGAL::copy_n(sphereRnd, numPoints, std::back_inserter(innerRandomPoints));

	std::list<KDPoint> randomPoints;
	std::list<KDPoint>::iterator pointsIter;
	for(pointsIter = innerRandomPoints.begin(); pointsIter != innerRandomPoints.end(); ++pointsIter)
	{
		KDPoint point = *pointsIter;
		KDVector vPoint1 = KDVector(point.x(), point.y(), point.z());
		KDVector vPoint1Unit = vPoint1 / CGAL::sqrt(vPoint1.squared_length());
		KDVector vPoint2 = vPoint1Unit * radius;
		randomPoints.push_back(KDPoint(vPoint2.x(), vPoint2.y(), vPoint2.z()));
	}
	return randomPoints;
	*/

	CGAL::Random_points_on_sphere_3<KDPoint, Pt_creator> sphereRnd(radius);
	std::list<KDPoint> randomPoints;
	CGAL::copy_n(sphereRnd, numPoints, std::back_inserter(randomPoints));
	return randomPoints;
}

std::list<KDPoint> randomPointsInCube(int numPoints, double radius)
{
	CGAL::Random_points_in_cube_3<KDPoint, Pt_creator> cubeRnd(radius);
	std::list<KDPoint> randomPoints;
	CGAL::copy_n(cubeRnd, numPoints, std::back_inserter(randomPoints));
	return randomPoints;
}

std::list<KDPoint> randomPointsInBox(int numPoints, KDPoint min, KDPoint max)
{
	CGAL::Random_points_in_cube_3<KDPoint, Pt_creator> cubeRnd(1.0);
	std::list<KDPoint> randomPointsCube;
	CGAL::copy_n(cubeRnd, numPoints, std::back_inserter(randomPointsCube));

	std::list<KDPoint> randomPoints;
	std::list<KDPoint>::iterator pointsIter;
	for(pointsIter = randomPointsCube.begin(); pointsIter != randomPointsCube.end(); ++pointsIter)
	{
		KDPoint point = *pointsIter;

		KDTransformation translate(CGAL::TRANSLATION, KDVector(1.0, 1.0, 1.0));
		KDTransformation scale(CGAL::SCALING, 0.5);
		point = translate(point);
		point = scale(point);

		KDTransformation scaleToBox((max.x()-min.x()), 0, 0, 0, (max.y()-min.y()), 0, 0, 0, (max.z()-min.z()));
		KDTransformation translateToBox(CGAL::TRANSLATION, KDVector(min.x(), min.y(), min.z()));
		point = scaleToBox(point);
		point = translateToBox(point);

		randomPoints.push_back(point);
	}
	return randomPoints;
}

std::pair<Point,Point> getMeshBoundingBox(MeshData &meshData)
{
	std::list<Triangle> meshTriangles = meshData.first;
	std::list<Point> pointsInMesh;
	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = meshTriangles.begin(); triangleIter != meshTriangles.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		pointsInMesh.push_back(t.vertex(0));
		pointsInMesh.push_back(t.vertex(1));
		pointsInMesh.push_back(t.vertex(2));
	}

	Kernel::Iso_cuboid_3 isoCuboid = CGAL::bounding_box(pointsInMesh.begin(), pointsInMesh.end());

	return std::pair<Point,Point>(isoCuboid.min(), isoCuboid.max());
}

//TODO: use CGAL::Cartesian_converter<K1,K2> converter;
//K1::Point_3 p_k1(1,2,3);
//K2::Point_3 p_k2 = converter( p_k1 );
KDPoint converPointExactToInexact(Point point)
{
	return KDPoint(CGAL::to_double(point.x()), CGAL::to_double(point.y()), CGAL::to_double(point.z()));
}

Point converPointInexactToExact(KDPoint point)
{
	return Point(CGAL::to_double(point.x()), CGAL::to_double(point.y()), CGAL::to_double(point.z()));
}

Point getMeshBarycenter(MeshData &meshData)
{
	std::list<Triangle> *meshTriangles = &(meshData.first);
	std::list<KDTriangle> meshTrianglesInexact;
	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = meshTriangles->begin(); triangleIter != meshTriangles->end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		meshTrianglesInexact.push_back(KDTriangle(converPointExactToInexact(t.vertex(0)), converPointExactToInexact(t.vertex(1)), converPointExactToInexact(t.vertex(2)) ));

	}

	KDPoint barycenter = CGAL::centroid(meshTrianglesInexact.begin(), meshTrianglesInexact.end(), CGAL::Dimension_tag<2>());
	return converPointInexactToExact(barycenter);
}

/*
//TODO: for transformation
Exact_predicates_exact_constructions_kernel implements a filtering
mechanism maintain a DAG of the cascaded constructions.
Each transformation creates a note in the DAG.

When the answer to a predicate can be certified using approximated
constructions, the DAG does not change. When approximations are
not sufficient to conclude (what we call a filter failure), a traversal
of the DAG is made and the corresponding path is simplified and replaced
by the corresponding exact value (memory consumption usually decreases).

In general it is not a good idea to use this kernel if you plan to do
cascaded constructions.

If gmp is available on your machine, you can try using
CGAL::Cartesian<CGAL::Gmpq> (reference counted objects with Gmpq as
number type) or CGAL::Cartesian<CGAL::Lazy_exact_nt<CGAL::Gmpq> >
(reference counted objects with CGAL::Lazy_exact_nt<CGAL::Gmpq> which is
a filtered number type).
*/

MeshData translateMesh(MeshData &meshData, Point point)
{
	std::list<Triangle> *meshTriangles = &(meshData.first);
	//std::list<TriangleInfo> cutInfo = meshData.second;

	std::list<Triangle> meshTrianglesTranslated;
	CGAL::Aff_transformation_3<Kernel> translate(CGAL::TRANSLATION, Vector(point.x(), point.y(), point.z()));

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = meshTriangles->begin(); triangleIter != meshTriangles->end(); ++triangleIter)
	{
		//Method 1
		//Triangle t = *triangleIter;
		//Triangle tTranslated(translate(t.vertex(0)), translate(t.vertex(1)), translate(t.vertex(2)) );
		//meshTrianglesTranslated.push_back(tTranslated);

		//Method 2
		meshTrianglesTranslated.push_back(triangleIter->transform(translate));
	}

	//return MeshData(meshTrianglesTranslated, cutInfo);
	return MeshData(meshTrianglesTranslated, meshData.second);
}

MeshData scaleMesh(MeshData &meshData, double scaleFactor)
{

	std::list<Triangle> *meshTriangles = &(meshData.first);
	//std::list<TriangleInfo> cutInfo = meshData.second;

	std::list<Triangle> meshTrianglesScaled;
	CGAL::Aff_transformation_3<Kernel> scale(CGAL::SCALING, scaleFactor);

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = meshTriangles->begin(); triangleIter != meshTriangles->end(); ++triangleIter)
	{
		//Method 1
		//Triangle t = *triangleIter;
		//Triangle tScaled(scale(t.vertex(0)), scale(t.vertex(1)), scale(t.vertex(2)) );
		//meshTrianglesScaled.push_back(tScaled);

		//Method 2
		meshTrianglesScaled.push_back(triangleIter->transform(scale));

		//Method 3: same memory problem of method 2
		//Point v0(triangleIter->vertex(0).x()*scaleFactor, triangleIter->vertex(0).y()*scaleFactor, triangleIter->vertex(0).z()*scaleFactor );
		//Point v1(triangleIter->vertex(1).x()*scaleFactor, triangleIter->vertex(1).y()*scaleFactor, triangleIter->vertex(1).z()*scaleFactor );
		//Point v2(triangleIter->vertex(2).x()*scaleFactor, triangleIter->vertex(2).y()*scaleFactor, triangleIter->vertex(2).z()*scaleFactor );
		//meshTrianglesScaled.push_back(Triangle(v0, v1, v2));
	}

	//return MeshData(meshTrianglesScaled, cutInfo);
	return MeshData(meshTrianglesScaled, meshData.second);
}

TrianglesList scaleAndCenterMesh(TrianglesList &meshTriangles, double scaleFactor)
{
	//Build MeshData
	TrianglesInfoList cutInfo = createNewTriangleInfoList(meshTriangles);
	MeshData meshData(meshTriangles, cutInfo);

	//Center mesh
	std::list<MeshData> listMeshData;
	listMeshData.push_back(meshData);
	std::list<TranslatedMeshData> translatedMeshData = centerMeshesInBarycenter(listMeshData);

	//Scale mesh
	MeshData scaledMeshData = scaleMesh(translatedMeshData.front().first, scaleFactor);

	return scaledMeshData.first;
}

//Find the barycenter for each mesh and translate each mesh into the barycenter, returning mesh and barycenter
std::list<TranslatedMeshData> centerMeshesInBarycenter(std::list<MeshData> &listMeshData)
{

	std::list< std::pair<MeshData,Point> > result;
	std::list<MeshData>::iterator meshDataInter;
	for(meshDataInter = listMeshData.begin(); meshDataInter != listMeshData.end(); ++meshDataInter)
	{
		MeshData meshData = *meshDataInter;
		Point barycenter = getMeshBarycenter(meshData);
		MeshData meshDataTranslated = translateMesh(meshData, Point(-barycenter.x(), -barycenter.y(), -barycenter.z()));
		result.push_back(TranslatedMeshData(meshDataTranslated, barycenter));
	}

	return result;
}

std::list<KDPoint> translatePoints(std::list<KDPoint> &listPoints, KDPoint vectorPoint)
{
	std::list<KDPoint> translatedPoints;
	KDTransformation translate(CGAL::TRANSLATION, KDVector(vectorPoint.x(), vectorPoint.y(), vectorPoint.z()));
	std::list<KDPoint>::iterator vectorPointInter;
	for(vectorPointInter = listPoints.begin(); vectorPointInter != listPoints.end(); ++vectorPointInter)
	{
		KDPoint point = *vectorPointInter;
		translatedPoints.push_back(translate(point));
	}
	return translatedPoints;
}

#ifdef DELAUNAYVORONOI_USE_TBB
tbb::mutex mutexforDelaunayVoronoiTbb;
//TBB operator class
class TBBVoronoiCell {

	MeshData *const meshData;
	Delaunay *const T;
	//tbb::concurrent_vector<MeshData> *concurrentResultMeshdata;
	std::list<MeshData> *const resultMeshdata;

	public:
		//TBBVoronoiCell(MeshData &md, Delaunay &dT, tbb::concurrent_vector<MeshData> &crm)
		TBBVoronoiCell(MeshData &md, Delaunay &dT, std::list<MeshData> &rm) : meshData(&md), T(&dT), resultMeshdata(&rm) { }

		//~TBBVoronoiCell() {
		//	std::cout << "  Call of ~TBBVoronoiCell" << std::endl;
		//}

		void operator()( DVertex_handle vh ) const {
			//std::cout << "TBBVoronoiCell::operator()" << std::endl;

			mutexforDelaunayVoronoiTbb.lock();

			//The direct assign of cutMeshTriangles = meshData->first causes a crash on multiple calls of voronoiShatter,
			// due to the copy of the Triangle. In order to work correctly, the Triangle are recreated
			//std::list<Triangle> cutMeshTriangles = meshData->first;
			std::list<TriangleInfo> cutInfo = meshData->second;
			std::list<Triangle> cutMeshTriangles;
			//std::list<TriangleInfo> cutInfo;
			std::list<Triangle>::iterator triangleIter;
			for(triangleIter=meshData->first.begin(); triangleIter!= meshData->first.end(); ++triangleIter)
			{
				Triangle t = *triangleIter;
				cutMeshTriangles.push_back(Triangle(t.vertex(0),t.vertex(1),t.vertex(2))); //See the comment above
			}
			//std::list<TriangleInfo>::iterator triangleInfoIter;
			//for(triangleInfoIter=meshData->second.begin(); triangleInfoIter!= meshData->second.end(); ++triangleInfoIter)
			//{
			//	TriangleInfo ti = *triangleInfoIter;
			//	cutInfo.push_back(ti);
			//}

			std::list<DEdge> segs;
			T->finite_incident_edges(vh, std::back_inserter(segs));
			mutexforDelaunayVoronoiTbb.unlock();
			std::cout << "  num edges: " << segs.size() << std::endl;

			std::list<DEdge>::iterator segsIter;
			for(segsIter=segs.begin(); segsIter!= segs.end(); ++segsIter)
			{
				DEdge edg = *segsIter;
				mutexforDelaunayVoronoiTbb.lock();
				DSegment_3 segment = T->segment(edg);
				mutexforDelaunayVoronoiTbb.unlock();
				KDVector vect = segment.to_vector();
				KDPoint p1 = segment.source();
				KDPoint p2 = segment.target();

				//KDPoint mp( (p1.x() + p2.x())/2.0, (p1.y() + p2.y())/2.0, (p1.z() + p2.z())/2.0);
				//Plane planeCutter_voronoi(Point(mp.x(), mp.y(), mp.z()), Vector(-vect.x(), -vect.y(), -vect.z()));
				//TODO: potrebbe esserci un errore a causa dei punti non esatti p1 e p2
				Point mp( (p1.x() + p2.x())/2.0, (p1.y() + p2.y())/2.0, (p1.z() + p2.z())/2.0);
				Plane planeCutter_voronoi(mp, Vector(-vect.x(), -vect.y(), -vect.z()));

				MeshData cutData = cutMesh(cutMeshTriangles, planeCutter_voronoi, cutInfo);
				cutMeshTriangles = cutData.first;
				cutInfo = cutData.second;

				//std::cout << "    temp mesh size: " << cutMeshTriangles.size() << std::endl;
			}

			mutexforDelaunayVoronoiTbb.lock();
			if (cutMeshTriangles.size() > 0)
			{
				//concurrentResultMeshdata->push_back(MeshData(cutMeshTriangles, cutInfo));
				resultMeshdata->push_back(MeshData(cutMeshTriangles, cutInfo));
			}
			else
			{
				std::cout << "    Warning: Final Voronoi mesh size empty!!" << std::endl;
			}
			mutexforDelaunayVoronoiTbb.unlock();

		}

};

//TODO: still sometimes is crashing: test on linux and using a different compiler or TBB libs
std::list<MeshData> voronoiShatter(MeshData &meshData, std::list<KDPoint> points)
{
	CGAL::Timer timer;
	timer.start();

	std::list<MeshData> resultMeshdata;
	//tbb::concurrent_vector<MeshData> concurrentResultMeshdata;
	Delaunay T(points.begin(), points.end());
	std::cout << "T.number_of_vertices:" <<  T.number_of_vertices() << std::endl;
	std::cout << "T.number_of_edges:" <<  T.number_of_finite_edges() << std::endl;
	std::cout << "T.is_valid:" <<  T.is_valid() << std::endl;
	std::vector<DVertex_handle> vertexHandleVector;
	Delaunay::Finite_vertices_iterator vit;
	for (vit = T.finite_vertices_begin(); vit != T.finite_vertices_end(); ++vit)
	{
		DVertex_handle vh = vit;
		vertexHandleVector.push_back(vh);
	}

	//Run multiple Thread using TBB
	//TBBVoronoiCell tbbVC(meshData, T, concurrentResultMeshdata);
	TBBVoronoiCell tbbVC(meshData, T, resultMeshdata);
	int numberThreads = tbb::task_scheduler_init::default_num_threads();
	std::cout << "numberThreads: " << numberThreads << std::endl;
	tbb::task_scheduler_init init(numberThreads);
	//tbb::parallel_do(T.finite_vertices_begin(), T.finite_vertices_end(), tbbVC);
	tbb::parallel_do(vertexHandleVector.begin(), vertexHandleVector.end(), tbbVC);

	/*
	//Build result
	std::cout << "Building result..." << std::endl;
	tbb::concurrent_vector<MeshData>::iterator crmi;
	for (crmi = concurrentResultMeshdata.begin(); crmi != concurrentResultMeshdata.end(); ++crmi)
	{
		//MeshData md = *crmi;
		resultMeshdata.push_back(*crmi);
	}
	*/

	timer.stop();
	std::cout << "Total tbb voronoiShatter construction took " << timer.time() << " seconds, total cells:" << resultMeshdata.size() << std::endl;

	return resultMeshdata;
}
#else
std::list<MeshData> voronoiShatter(MeshData &meshData, std::list<KDPoint> points)
{
	CGAL::Timer timer;
	timer.start();

	std::list<MeshData> resultMeshdata;

	Delaunay T(points.begin(), points.end());
	std::cout << "T.number_of_vertices:" <<  T.number_of_vertices() << std::endl;
	std::cout << "T.number_of_edges:" <<  T.number_of_finite_edges() << std::endl;
	std::cout << "T.is_valid:" <<  T.is_valid() << std::endl;

	int countNumberVertices = 0; //Just to count
	Delaunay::Finite_vertices_iterator vit;
	for (vit = T.finite_vertices_begin(); vit != T.finite_vertices_end(); ++vit)
	{
		DVertex_handle vh = vit;

		std::list<Triangle> cutMeshTriangles = meshData.first;
		std::list<TriangleInfo> cutInfo = meshData.second;

		std::list<DEdge> segs;
		T.finite_incident_edges(vh, std::back_inserter(segs));
		std::cout << " (" << countNumberVertices++ << ")  num edges: " << segs.size() << std::endl;

		std::list<DEdge>::iterator segsIter;
		for(segsIter=segs.begin(); segsIter!= segs.end(); ++segsIter)
		{
			DEdge edg = *segsIter;
			DSegment_3 segment = T.segment(edg);
			KDVector vect = segment.to_vector();
			KDPoint p1 = segment.source();
			KDPoint p2 = segment.target();

			//KDPoint mp( (p1.x() + p2.x())/2.0, (p1.y() + p2.y())/2.0, (p1.z() + p2.z())/2.0);
			//Plane planeCutter_voronoi(Point(mp.x(), mp.y(), mp.z()), Vector(-vect.x(), -vect.y(), -vect.z()));
			//TODO: potrebbe esserci un errore a causa dei punti non esatti p1 e p2
			Point mp( (p1.x() + p2.x())/2.0, (p1.y() + p2.y())/2.0, (p1.z() + p2.z())/2.0);
			Plane planeCutter_voronoi(mp, Vector(-vect.x(), -vect.y(), -vect.z()));

			MeshData cutData = cutMesh(cutMeshTriangles, planeCutter_voronoi, cutInfo);
			cutMeshTriangles = cutData.first;
			cutInfo = cutData.second;

			//std::cout << "    temp mesh size: " << cutMeshTriangles.size() << std::endl;

			//TODO: change TriangleInfo adding:
			// int meshId, meshIdAdjacent
			// and populate these new two variable for the triangles into the cut plane;
			// each mesh piece will have a different id;
			// these two new variable could be used into the RejointMeshes algorithm instead of reportAdjacentIntersectionCallback:
			// knowing this ids it is very easy remove the the triangles in shared cut surfaces

		}

		if (cutMeshTriangles.size() > 0)
		{
			resultMeshdata.push_back(MeshData(cutMeshTriangles, cutInfo));
		}
		else
		{
			std::cout << "    Warning: Final Voronoi mesh size empty!!" << std::endl;
		}
	}

	timer.stop();
	std::cout << "Total voronoiShatter construction took " << timer.time() << " seconds, total cells:" << resultMeshdata.size() << std::endl;

	return resultMeshdata;
}
#endif

std::list<MeshData> voronoiShatter_uniformDistributionPoints(TrianglesList &meshTriangles, int numPoints, bool doDisjointMesh)
{
	//Build MeshData
	TrianglesInfoList cutInfo = createNewTriangleInfoList(meshTriangles);
	MeshData meshData(meshTriangles, cutInfo);
	return voronoiShatter_uniformDistributionPoints(meshData, numPoints, doDisjointMesh);
}

//Completed Voronoi Shatter distribution function: uniformDistributionPoints
std::list<MeshData> voronoiShatter_uniformDistributionPoints(MeshData &meshData, int numPoints, bool doDisjointMesh)
{
	//Get Bounding Box
	std::pair<Point,Point> boundingBox = getMeshBoundingBox(meshData);
	KDPoint bboxMin = converPointExactToInexact(boundingBox.first);
	KDPoint bboxMax = converPointExactToInexact(boundingBox.second);

	//Generate random points
	std::list<KDPoint> randomPoints = randomPointsInBox(numPoints, bboxMin, bboxMax);


	//------------ TEST SAVED POINTS -------------
	/*
	//Save Points for test
	std::ofstream out("voronoi_random_points_test_benchmark.ser", std::ios::binary);
	std::list<KDPoint>::iterator pointsIterator;
	for(pointsIterator = randomPoints.begin(); pointsIterator != randomPoints.end(); ++pointsIterator)
	{
		KDPoint ppp = *pointsIterator;
		//std::cout << "Punto Random save: " << ppp.x() << ", " << ppp.y() << ", " << ppp.z() << std::endl;
		out.write((char*)&ppp, sizeof(ppp));
	}
	out.close();
	 */
	/*
	//Load Points for test
	//std::list<KDPoint> randomPoints;
	randomPoints.clear();
	std::ifstream in("voronoi_random_points_test.ser", std::ios::binary);
	for (unsigned int i = 0; i < 5; i++)
	{
		KDPoint temp;
		in.read((char*)&temp, sizeof(temp));
		randomPoints.push_back(temp);
	}
	in.close();
	*/
	//------------ TEST SAVED POINTS -------------


	//Compute Voronoi Shatter
	std::list<MeshData> shatteredMeshes = voronoiShatter(meshData, randomPoints);

	if (doDisjointMesh)
	{
		//Disjoint meshes
		shatteredMeshes = disjointNonContiguousListMeshes(shatteredMeshes);
	}

	//Translate generated shattered meshes in barycenter
		//std::list<TranslatedMeshData> translatedMeshData = centerMeshesInBarycenter(shatteredMeshes); //This is moved out this function
		//return translatedMeshData;

	return shatteredMeshes;
}

std::list<MeshData> voronoiShatter_sphereDistributionOnPoint(TrianglesList &meshTriangles, int numPoints, KDPoint targetPoint, double radius, bool doDisjointMesh)
{
	//Build MeshData
	TrianglesInfoList cutInfo = createNewTriangleInfoList(meshTriangles);
	MeshData meshData(meshTriangles, cutInfo);

	return voronoiShatter_sphereDistributionOnPoint(meshData, numPoints, targetPoint, radius, doDisjointMesh);
}

std::list<MeshData> voronoiShatter_sphereDistributionOnPoint(MeshData &meshData, int numPoints, KDPoint targetPoint, double radius, bool doDisjointMesh)
{
	//Generate random points in a sphere and translate in targetPoint
	//std::list<KDPoint> randomPoints = randomPointsInSphere(numPoints, radius);
	#define GAUSS_WIDTH 1.0
	std::list<KDPoint> randomPoints = randomPointsInSphereGaussianDistribution(numPoints, radius, GAUSS_WIDTH);
	std::list<KDPoint> translatedPoints = translatePoints(randomPoints, targetPoint);

	//Compute Voronoi Shatter
	std::list<MeshData> shatteredMeshes = voronoiShatter(meshData, translatedPoints);

	if (doDisjointMesh)
	{
		//Disjoint meshes
		shatteredMeshes = disjointNonContiguousListMeshes(shatteredMeshes);
	}

	//Translate generated shattered meshes in barycenter
		//std::list<TranslatedMeshData> translatedMeshData = centerMeshesInBarycenter(shatteredMeshes); //This is moved out this function
		//return translatedMeshData;

	return shatteredMeshes;
}

std::list<MeshData> disjointNonContiguousListMeshes(std::list<MeshData> &listMeshData)
{
	std::list<MeshData> result;

	//cycle listMeshData and build the result
	std::list<MeshData>::iterator meshDataInter;
	for(meshDataInter = listMeshData.begin(); meshDataInter != listMeshData.end(); ++meshDataInter)
	{
		MeshData meshData = *meshDataInter;

		//Disjoint mesh
		std::list<MeshData> disjointMeshes = disjointNonContiguousMeshes(meshData);

		std::list<MeshData>::iterator disjointMeshIter;
		for(disjointMeshIter = disjointMeshes.begin(); disjointMeshIter != disjointMeshes.end(); ++disjointMeshIter)
		{
			MeshData pieceMesh = *disjointMeshIter;
			result.push_back(pieceMesh);
		}
	}

	return result;
}

//Call the rejointMesh in RejointMeshes.cpp on the meshes that have distance with targetPoint greater than double distance
// use isExternJoin = true to joint the extern meshes, use isExternJoin = false to joint the internal meshes
std::list<MeshData> rejointMeshesInDistance(std::list<MeshData> &listMeshData, KDPoint targetPoint, double distance, double isExternJoin)
{
	std::list<MeshData> result;
	std::list<MeshData> meshesToJoin;
	double squaredDistance = distance * distance;
	Point exactTargetPoint(targetPoint.x(), targetPoint.y(), targetPoint.z());

	std::list<MeshData>::iterator meshDataInter;
	for(meshDataInter = listMeshData.begin(); meshDataInter != listMeshData.end(); ++meshDataInter)
	{
		bool meshIsOutOfDistance = true; //Algorithm 1
		//bool meshIsOutOfDistance = false; //Algorithm 2
		std::list<Triangle>::iterator triangleIter;
		for(triangleIter = meshDataInter->first.begin(); triangleIter != meshDataInter->first.end(); ++triangleIter)
		{
			Triangle t = *triangleIter;

			//Algorithm 1: This algorithm excludes from the join the meshes on the radius
			for (int i = 0; i < 3; ++i)
			{
				Point tv = t.vertex(i);

				//it could be used a logic XNOR but it is not so understable, so I use this condition
				if (isExternJoin)
				{
					if (CGAL::squared_distance(tv, exactTargetPoint) < squaredDistance)
					{
						meshIsOutOfDistance = false;
					}
				}
				else
				{
					if (CGAL::squared_distance(tv, exactTargetPoint) > squaredDistance)
					{
						meshIsOutOfDistance = false;
					}
				}
			}
			if (!meshIsOutOfDistance)
			{
				break;
			}


			/*
			//Algorithm 2: This algorithm includes from the join the meshes on the radius
			//Ideale per pezzi piccoli e dominio dei punti voronoi dentro il raggio,
			// sembra funzionare solo per isExternJoin, TODO: test
			for (int i = 0; i < 3; ++i)
			{
				Point tv = t.vertex(i);

				if (isExternJoin)
				{
					if (CGAL::squared_distance(tv, exactTargetPoint) > squaredDistance)
					{
						meshIsOutOfDistance = true;
					}
				}
				else
				{
					if (CGAL::squared_distance(tv, exactTargetPoint) < squaredDistance)
					{
						meshIsOutOfDistance = true;
					}
				}
			}
			if (meshIsOutOfDistance)
			{
				break;
			}
			*/

		}

		if (meshIsOutOfDistance)
		{
			meshesToJoin.push_back(*meshDataInter);
		}
		else
		{
			result.push_back(*meshDataInter);
		}
	}

	if (meshesToJoin.size() > 0)
	{
		MeshData rejointMeshes = simpleRejointMeshes(meshesToJoin);
		//MeshData rejointMeshes = rejointMeshes(meshesToJoin);
		result.push_back(rejointMeshes);
	}

	std::cout << "Executed rejointMeshesInDistance: not joined meshes: " << (listMeshData.size() - meshesToJoin.size()) << ", joined meshes: " << meshesToJoin.size() << std::endl;

	return result;

}

//template <class SerializeType>
void saveTranslatedMeshDataList(std::list<TranslatedMeshData> &transMesh, const char * filename, const int fileType)
{
	if (fileType == DELAUNAYVORONOI_BINARY_FILE_TYPE)
	{
		std::ofstream fileIn(filename, std::ios::out | std::ios::binary);
		//boost::archive::binary_oarchive oa(fileIn); //take a look into the boost::serialization::save template block in DelaunayVoronoi.h
		boost::archive::binary_oarchive_pointsmap oa(fileIn);
		oa & transMesh;
		fileIn.close();
	}
	else if (fileType == DELAUNAYVORONOI_TEXT_FILE_TYPE)
	{
		std::ofstream fileIn(filename, std::ios::out);
		//boost::archive::text_oarchive oa(fileIn); //take a look into the boost::serialization::save template block in DelaunayVoronoi.h
		boost::archive::text_oarchive_pointsmap oa(fileIn);
		oa & transMesh;
		fileIn.close();
	}
	else
	{
		std::cout << "   Error: Unsuppoerted file type!" << std::endl;
	}
}

//template <class SerializeType>
std::list<TranslatedMeshData> loadTranslatedMeshDataList(const char * filename, const int fileType)
{
	//SerializeType transMesh;
	std::list<TranslatedMeshData> transMesh;

	if (fileType == DELAUNAYVORONOI_BINARY_FILE_TYPE)
	{
		std::ifstream fileOut(filename, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive ia(fileOut);
		ia & transMesh;
		fileOut.close();
	}
	else if (fileType == DELAUNAYVORONOI_TEXT_FILE_TYPE)
	{
		std::ifstream fileOut(filename, std::ios::in);
		boost::archive::text_iarchive ia(fileOut);
		ia & transMesh;
		fileOut.close();
	}
	else
	{
		std::cout << "   Error: Unsuppoerted file type!" << std::endl;
	}
	return transMesh;
}

namespace boost {
	namespace archive {

		TripleDouble PointsMapperArchive::getMappedPoint(Point p)
		{
			std::map<Point, TripleDouble>::iterator foundPair = mapPointsDoubleValues.find(p);
			if (foundPair == mapPointsDoubleValues.end())
			{
				double x = CGAL::to_double(p.x());
				double y = CGAL::to_double(p.y());
				double z = CGAL::to_double(p.z());
				TripleDouble tripleDouble;
				tripleDouble.d1 = x;
				tripleDouble.d2 = y;
				tripleDouble.d3 = z;
				mapPointsDoubleValues.insert(std::pair<Point, TripleDouble>(p, tripleDouble));
				return tripleDouble;
			}
			else
			{
				return foundPair->second;
			}
		}

		text_oarchive_pointsmap::text_oarchive_pointsmap(std::ofstream &ofs) : text_oarchive(ofs)
		{
			std::cout << "   Saving points using PointsMapperArchive..." << std::endl;
		}

		binary_oarchive_pointsmap::binary_oarchive_pointsmap(std::ofstream &ofs) : binary_oarchive(ofs)
		{
			std::cout << "   Saving points using PointsMapperArchive..." << std::endl;
		}

	}
}


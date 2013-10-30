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

#include "CutterMesher.h"

#include <CGAL/exceptions.h>

#ifdef CUTTERMESHER_TEST_DEBUG
//Solo per il test
#include "LinesViewer.h"
	VectListCSegment2d vettorelisteseg;
#endif

inline bool isSignedDistancePositive(Plane &plane, Point point)
{
	//Se il punto e' sul piano questa funzione impiega molto piu' tempo per restituire un risultato
	return compare_signed_distance_to_plane(plane, plane.point(), point) == CGAL::SMALLER;
}

inline TriangleInfo getNewTriangleInfo(short int cutType)
{
	TriangleInfo tInfo;
	tInfo.cutType = cutType;
	return tInfo;
}

void addInTrianglesInfoOutput(std::list<TriangleInfo> &trianglesInfoOutput, MapIdTriangleInfo &mapIdTriangleInfo, Triangle &t, short int cutType)
{

	if (cutType == CUTTERMESHER_TRIANGLE_NO_CUTTED)
	{
		//Cerco t.id() in mapIdTriangleInfo
		MapIdTriangleInfo::iterator foundTriangleInfo;
		if ( ( foundTriangleInfo = mapIdTriangleInfo.find(t.id())) != mapIdTriangleInfo.end())
		{
			trianglesInfoOutput.push_back(getNewTriangleInfo((foundTriangleInfo->second).cutType));
		}
		else
		{
			std::cout << "ERROR: foundTriangleInfo NOT FOUND :CUTTERMESHER_TRIANGLE_NO_CUTTED" << std::endl;
		}
	}
	else if (cutType == CUTTERMESHER_TRIANGLE_CUTTED)
	{
		//Cerco t.id() in mapIdTriangleInfo
		MapIdTriangleInfo::iterator foundTriangleInfo;
		if ( ( foundTriangleInfo = mapIdTriangleInfo.find(t.id())) != mapIdTriangleInfo.end())
		{
			if (((foundTriangleInfo->second).cutType) == CUTTERMESHER_TRIANGLE_NO_CUTTED)
			{
				trianglesInfoOutput.push_back(getNewTriangleInfo(CUTTERMESHER_TRIANGLE_CUTTED));
			}
			else
			{
				trianglesInfoOutput.push_back(getNewTriangleInfo((foundTriangleInfo->second).cutType));
			}
		}
		else
		{
			std::cout << "ERROR: foundTriangleInfo NOT FOUND :CUTTERMESHER_TRIANGLE_CUTTED" << std::endl;
		}
	}
	else if (cutType == CUTTERMESHER_TRIANGLE_IN_CUT_PLANE)
	{
		trianglesInfoOutput.push_back(getNewTriangleInfo(CUTTERMESHER_TRIANGLE_IN_CUT_PLANE));
	}
	else
	{
		std::cout << "ERROR: WRONG CUT TYPE!!" << std::endl;
	}

}

std::list<TriangleInfo> createNewTriangleInfoList(std::list<Triangle> &meshInput)
{
	std::list<TriangleInfo> trianglesInfo;

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter=meshInput.begin(); triangleIter!= meshInput.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		trianglesInfo.push_back(getNewTriangleInfo(CUTTERMESHER_TRIANGLE_NO_CUTTED));
	}
	//Or just push_back meshInput.size() elements

	return trianglesInfo;
}

MeshData cutMesh(std::list<Triangle> &meshInput,  Plane plane, std::list<TriangleInfo> &trianglesInfoInput, bool useDelaunay, double bCriteria, double sCriteria) {

	CGAL::Timer timer;
	timer.start();

	#ifdef CUTTERMESHER_LOG
	std::cout << "meshInput total triangles: " << meshInput.size() << std::endl;
	#endif

	std::list<Triangle> meshOutput;
	std::list<TriangleInfo> trianglesInfoOutput;
	std::list<Triangle> intersectTriangles;
	std::list<Segment> intersectSegments;

	//error se diversi size in input
	if (meshInput.size() != trianglesInfoInput.size())
	{
		std::cout << "ERROR: meshInput.size() != trianglesInfoInput.size()" << std::endl;
		exit(-1);
	}

	//return if meshInput is empty
	if (meshInput.size() == 0)
	{
		return MeshData(meshOutput, trianglesInfoOutput);
	}

	//Creo la mappa triangle.id--> *TriangleInfo in trianglesInfoInput e la popolo
	#ifdef CUTTERMESHER_LOG
	std::cout << "Build MapIdTriangleInfo" << std::endl;
	#endif
	MapIdTriangleInfo mapIdTriangleInfo;
	std::list<Triangle>::iterator triangleIterForBuildMap;
	std::list<TriangleInfo>::iterator triangleInfoIterForBuildMap;
	for(triangleIterForBuildMap=meshInput.begin(), triangleInfoIterForBuildMap=trianglesInfoInput.begin();
			triangleIterForBuildMap!= meshInput.end();
			++triangleIterForBuildMap, ++triangleInfoIterForBuildMap)
	{
		Triangle t = *triangleIterForBuildMap;
		TriangleInfo tInfo = *triangleInfoIterForBuildMap;
		mapIdTriangleInfo.insert(PairIdTriangleInfo(t.id(), tInfo));
	}

	//Aggiungo nella mesh solo i triangoli che hanno tutti i vertici con distanza dal piano positiva
	#ifdef CUTTERMESHER_LOG
	std::cout << "Adding Triangles in positive plane and intersectTriangles" << std::endl;
	#endif
	std::list<Triangle>::iterator triangleIter;
	for(triangleIter=meshInput.begin(); triangleIter!= meshInput.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		bool isTrianglePointDistanceSmaller[3];
		isTrianglePointDistanceSmaller[0] = isSignedDistancePositive(plane, t.vertex(0));
		isTrianglePointDistanceSmaller[1] = isSignedDistancePositive(plane, t.vertex(1));
		isTrianglePointDistanceSmaller[2] = isSignedDistancePositive(plane, t.vertex(2));

		if (isTrianglePointDistanceSmaller[0] &&
			isTrianglePointDistanceSmaller[1] &&
			isTrianglePointDistanceSmaller[2] )
		{
			//Il triangolo e' tutto sopra il piano quindi lo aggiungo alla mesh di output
			meshOutput.push_back(t);
			//Add triangle info for cut type
			addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_NO_CUTTED);

		} else if (!isTrianglePointDistanceSmaller[0] &&
				!isTrianglePointDistanceSmaller[1] &&
				!isTrianglePointDistanceSmaller[2])
		{
			//Il triangolo e' tutto sotto il piano quindi lo scarto
		} else {
			//Il triangolo e' intersecato dal piano quindi aggiungo il triangolo a intersectTriangles
			intersectTriangles.push_back(t);
		}

	}

	#ifdef CUTTERMESHER_LOG
	std::cout << "Added Triangles in meshOutput: " << meshOutput.size() << std::endl;
	std::cout << "Added Triangles in intersectTriangles: " << intersectTriangles.size() << std::endl;
	#endif

	//Se il numero di triangoli su cui dovrei calcolare le intersezioni e' 0 allora ritorno solo i triangoli gia' aggiunti in meshOutput
	if (intersectTriangles.size() == 0)
	{
		return MeshData(meshOutput, trianglesInfoOutput);
	}

	//Patch for CGAL ERROR: assertion violation: Expr: m_primitives.size() > 1 in CGAL-4.1\include/CGAL/AABB_tree.h line 336
	if (intersectTriangles.size() == 1)
	{
		//build a triangle that is not intersecting the plane
		Point pointOnPlane1 = plane.point();
		Vector orthogonalVector = plane.orthogonal_vector();
		Point pointNotOnPlane1 = Point(pointOnPlane1.x() + orthogonalVector.x(),
									pointOnPlane1.y() + orthogonalVector.y(),
									pointOnPlane1.z() + orthogonalVector.z());
		Vector base1 = plane.base1();
		Vector base2 = plane.base2();
		Point pointNotOnPlane2 = Point(pointNotOnPlane1.x() + base1.x(),
									pointNotOnPlane1.y() + base1.y(),
									pointNotOnPlane1.z() + base1.z());
		Point pointNotOnPlane3 = Point(pointNotOnPlane1.x() + base2.x(),
									pointNotOnPlane1.y() + base2.y(),
									pointNotOnPlane1.z() + base2.z());
		intersectTriangles.push_back(Triangle(pointNotOnPlane1, pointNotOnPlane2, pointNotOnPlane3));
	}

	#ifdef CUTTERMESHER_LOG
	std::cout << "Building tree" << std::endl;
	#endif
	Tree tree(intersectTriangles.begin(), intersectTriangles.end());

	std::list<Object_and_primitive_id> intersections;
	#ifdef CUTTERMESHER_LOG
	std::cout << "Start tree.all_intersections" << std::endl;
	#endif
	tree.all_intersections(plane, std::back_inserter(intersections));
	#ifdef CUTTERMESHER_LOG
	std::cout << "End tree.all_intersections: total: " << intersections.size() << " intersections" << std::endl;
	#endif

	std::list<Object_and_primitive_id>::iterator intersIter;
	#ifdef CUTTERMESHER_LOG
	std::cout << "Start compute for intersections..." << std::endl;
	#endif
	for(intersIter=intersections.begin(); intersIter!= intersections.end(); ++intersIter)
	{

		Object_and_primitive_id op = *intersIter;

		//Get intersection primitive
		Primitive id = op.second;
		Triangle t = id.datum();

		//Get intersection points
		CGAL::Object object = op.first;
		Segment segment;
		Point point;
		Triangle triangle;
		if(CGAL::assign(segment,object))
		{
			//Aggiungo i segmenti per il bordo
			intersectSegments.push_back(segment);
		}
		else if(CGAL::assign(point,object))
		{
			//std::cout << "WARNING: intersection object is a Point" << std::endl;
			//Se l'oggetto di intersezione e' un punto allora deve essere il vertice di un triangolo che devo
			// aggiungere in meshOutput altrimenti viene escluso dalla striscia di intersezione
			meshOutput.push_back(t);
			//Add triangle info for cut type
			addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_NO_CUTTED);

			#ifdef CUTTERMESHER_LOG
			std::cout << "INFO: intersection object is a point" << std::endl;
			#endif
			continue;
		}
		else if(CGAL::assign(triangle,object))
		{
			#ifdef CUTTERMESHER_LOG
			std::cout << "WARNING: intersection object is a triangle" << std::endl;
			#endif
			continue;
		}
		else
		{
			#ifdef CUTTERMESHER_LOG
			std::cout << "WARNING: intersection object is NOT segment or point" << std::endl;
			#endif
			continue;
		}

		//Ricostruzione striscia di intersezione
		int numPositivePoints = 0;
		int indexVertexNegativePoint = -1;
		Point positivePoints[3];

		for (int i = 0;  i < 3; ++i)
		{
			Point triangleVertex = t.vertex(i);
			//std::cout << triangleVertex.id() <<" " << t.vertex(i).id() << " " << (triangleVertex == t.vertex(i)) << "\n";
			if (isSignedDistancePositive(plane, triangleVertex))
			{
				positivePoints[numPositivePoints] = triangleVertex;
				numPositivePoints++;
			} else {
				indexVertexNegativePoint = i;
			}
		}

		//std::cout << "numPositivePoints: " <<  numPositivePoints << std::endl;
		if (numPositivePoints == 3) {

			#ifdef CUTTERMESHER_LOG
			std::cout << "WARNING: numPositivePoints: " <<  numPositivePoints << std::endl;
			#endif
			continue;

		} else if (numPositivePoints == 1) {

			Triangle triangleToAdd(positivePoints[0],segment.start(),segment.end());
			meshOutput.push_back(triangleToAdd);
			//Add triangle info for cut type
			addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_CUTTED);

		} else if (numPositivePoints == 2) {

			//Se sono qui ci deve essere un solo vertice nella parte
			// negativa del piano corrispondente all'indice indexVertexNegativePoint
			Point vertexNegative = t.vertex(indexVertexNegativePoint);
			Segment triangleSide(vertexNegative, positivePoints[0]);
			FT distance1 = squared_distance(triangleSide,  segment.start());
			FT distance2 = squared_distance(triangleSide,  segment.end());
			//std::cout << distance1 << "   " <<  distance2 << std::endl;

			//if (triangleSide.has_on(segment.start())) { //Non funziona a causa dell'errore float
			if ( distance1 < distance2) {

				//
				//    1---2
				//    | \ |
				//    |  \|
				//    3---4
				//
				Triangle triangleToAdd1(positivePoints[0], positivePoints[1], segment.start());
				Triangle triangleToAdd2(positivePoints[1], segment.start(), segment.end());
				meshOutput.push_back(triangleToAdd1);
				meshOutput.push_back(triangleToAdd2);
				//Add triangle info for cut type
				addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_CUTTED);
				addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_CUTTED);

			//} else if (triangleSide.has_on(segment.end())) { //Non funziona a causa dell'errore float
			} else {

				Triangle triangleToAdd1(positivePoints[0], positivePoints[1], segment.end());
				Triangle triangleToAdd2(positivePoints[1], segment.start(), segment.end());
				meshOutput.push_back(triangleToAdd1);
				meshOutput.push_back(triangleToAdd2);
				//Add triangle info for cut type
				addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_CUTTED);
				addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, t, CUTTERMESHER_TRIANGLE_CUTTED);

			}

		} //End if costruzione triangoli striscia di intersezione

	} //End for intersections


	//Costruisco la faccia tagliata

	//Vettore di segmenti 2d del contorno di intersezione mesh-piano
	std::vector<Segment_2> outlineSegements;

	#ifdef CUTTERMESHER_TEST_DEBUG
	ListCSegment2d listasegtest;
	ListSegment_2 listaSegs_2;
	#endif

	//Proietto i segmenti 3d in intersectSegments sul piano plane
	std::list<Segment>::iterator segmentIter;
	for (segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
	{
		Segment segment3d = *segmentIter;
		Point start3d = segment3d.start();
		Point end3d = segment3d.end();

		//Proietto il punto 3d sul piano e ottengo un punto 2d
		KPoint2 start = plane.to_2d(start3d);
		KPoint2 end = plane.to_2d(end3d);

		//Se il segmento non e' degenere lo aggiungo ad outlineSegements
		//if ((start.x()!=end.x()) || (start.y()!=end.y()))
		if ( start!=end )
		{
			//Segment_2 segment2d(Point_2(start.x(), start.y()), Point_2(end.x(), end.y()));
			Segment_2 segment2d(start, end);
			outlineSegements.push_back(segment2d);

			#ifdef CUTTERMESHER_TEST_DEBUG
			CSegment2d seg2d;
			seg2d.startX =  CGAL::to_double(start.x());
			seg2d.startY =  CGAL::to_double(start.y());
			seg2d.endX =  CGAL::to_double(end.x());
			seg2d.endY =  CGAL::to_double(end.y());
			listasegtest.push_back(seg2d);
			listaSegs_2.push_back(segment2d);
			#endif

		}

	}

	#ifdef CUTTERMESHER_TEST_DEBUG
	vettorelisteseg.push_back(listasegtest);
	findIncongruencies(listaSegs_2);
	#endif

	//computo la gerarchia delle facce
	#ifdef CUTTERMESHER_LOG
	std::cout << "outlineSegements total: " << outlineSegements.size() << std::endl;
	std::cout << "Start for getFacesHolesHierarchy" << std::endl;
	#endif
	SegmentsArrangement segmentsArrangment;
	std::vector<FaceHoleInfo> allFacesHoles = segmentsArrangment.getFacesHolesHierarchy(outlineSegements);
	#ifdef CUTTERMESHER_LOG
	std::cout << "End for getFacesHolesHierarchy: total:" << allFacesHoles.size() << " faces" << std::endl;
	#endif

	if (allFacesHoles.size() > 0)
	{
		if (useDelaunay)
		{
			//try {
				bool validDelaunay = faceBuilder_Delaunay(segmentsArrangment, meshOutput, trianglesInfoOutput, plane, mapIdTriangleInfo, bCriteria, sCriteria);
				if (!validDelaunay)
				{
					std::cout << "   faceBuilder_Delaunay is not valid: using faceBuilder_exact..." << std::endl;
					faceBuilder_exact(segmentsArrangment, meshOutput, trianglesInfoOutput, plane, mapIdTriangleInfo);
				}
			//}
			//catch (CGAL::Assertion_exception e) {
			//	std::cout << "\n\nERROR ON faceBuilder_Delaunay: " << e.message() << std::endl;
			//}
		}
		else
		{

			#ifdef CUTTERMESHER_USE_ONLY_EXACT_FACE_BUILDER
				faceBuilder_exact(segmentsArrangment, meshOutput, trianglesInfoOutput, plane, mapIdTriangleInfo);
			#else
				bool isValidCDT = faceBuilder_inexact(segmentsArrangment, meshOutput, trianglesInfoOutput, plane, mapIdTriangleInfo);
				if (!isValidCDT) {
					faceBuilder_exact(segmentsArrangment, meshOutput, trianglesInfoOutput, plane, mapIdTriangleInfo);
				}
			#endif
		}
	}
	else
	{
		#ifdef CUTTERMESHER_LOG
		std::cout << "WARNING!! getFacesHolesHierarchy return 0 faces" << std::endl;
		#endif
	}

	#ifdef CUTTERMESHER_LOG
	std::cout << "meshOutput total triangles: " << meshOutput.size() << std::endl;
	std::cout << "trianglesInfoOutput total triangles: " << trianglesInfoOutput.size() << std::endl;
	#endif

	timer.stop();
	#ifdef CUTTERMESHER_LOG_TIME
	std::cout << "Construction took " << timer.time() << " seconds." << std::endl;
	#endif

	//return meshOutput;
	return MeshData(meshOutput, trianglesInfoOutput);
}


bool faceBuilder_inexact(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo)
{
	//Recupero allFacesHolesVector dal SegmentsArrangement
	std::vector<FaceHoleInfo> allFacesHoles = segmentsArrangment.getAllFacesHolesVectorInObject();

	//Costruisco la mesh 2d e la proietto sul piano 3d e aggiunto i triangoli ad meshOutput

	//TODO:
	//Dal seed che appartiene ad una faccia (non ad un hole) e convertito in 3d sul piano,
	// se quel seed e' interno ad una superficie completamente chiusa rappresentata da meshInput
	// allora chiudo la superficie con la mesh calcolata di seguito

	//NOTE:
	//Per il calcolo della mesh posso anche usare la precisione CGAL::Simple_cartesian<double>
	// ma comunque proiettare i triangoli della mesh sul piano usando la precisione  CGAL::Exact_predicates_exact_constructions_kernel

	//Usando la precisione CGAL::Simple_cartesian<double> in alcuni casi si ha un errore:
	//terminate called after throwing an instance of 'CGAL::Precondition_exception'
	//  what():  CGAL ERROR: precondition violation!
	//Expr: orientation(p, f->vertex(ccw(li))->point(), f->vertex(cw(li))->point()) == LEFT_TURN
	//File: CGAL-3.9/include/CGAL/Triangulation_2.h
	//Line: 1134
	// Usando Exact_predicates_inexact_constructions_kernel l'errore non si presenta
	// Si potrebbe anche usare Exact_predicates_exact_constructions_kernel e cambiare radicalmente l'algoritmo di seguito

	//CDT Serve per la creazione della mesh triangoli, aggiungendo i segmenti delle facce
	CDT cdt;
	bool isValidCDT = true;
	//Lista che memorizza i seeds della mesh (punti in cui l'area non deve essere meshata)
	//Si puo' usare solo se viene usata la triangolazione Delaunay, sostituito dal metodo isInArrangment di SegmentsArrangement
	//std::list<Point2> list_of_seeds;

	//Mappa che mappa i punti non esatti in esatti, per la frontiera della mesh
	// dato che la mesh usa un kernel inesatto, ci sarebbe un errore sui punti della frontiera
	MapExactPoints mapExactPoints;

	//Mappa che mappa Point_2 in Vertex_handle, per non creare piu' volte i stessi punti inesatti della mesh,
	//serve solo quando dai segmenti dell'arrangment si inseriscono i constraint per la mesh,
	//non viene utilizzata quando dalla mesh si ricreano i punti 3d
	MapInexactPoints mapInexactPoints;

	//STEP 1
	//Ciclo una prima volta tutte le facce trovate nell'arrangement e creo solo una volta i punti inesatti e li memorizzo in
	//mapExactPoints e mapInexactPoints mappando i punti esatti
	for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

		//Aggiungo i segmenti per i constraint della mesh
		//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
		std::list<Segment_2>::iterator faceSegsIter;
		for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
		{
			//std::cout << "+";

			Segment_2 seg = *faceSegsIter;
			if (seg.start() != seg.end())
			{

				#ifdef CUTTERMESHER_TEST_DEBUG
				std::cout << "[" <<seg.start().x() << ", " << seg.start().y() << " ; ";
				std::cout << seg.end().x() << ", " << seg.end().y() << "] " << std::endl;

				ListCSegment2d listasegtest2;
				CSegment2d seg2d;
				seg2d.startX =  CGAL::to_double(seg.start().x());
				seg2d.startY =  CGAL::to_double(seg.start().y());
				seg2d.endX =  CGAL::to_double(seg.end().x());
				seg2d.endY =  CGAL::to_double(seg.end().y());
				listasegtest2.push_back(seg2d);
				vettorelisteseg.push_back(listasegtest2);
				#endif

				//controllo se i punti seg.start() o seg.end() sono in mapInexactPoints
				// se non ci sono aggiunto alla mappa mapInexactPoints il Vertex_handle creato
				// e alla mappa mapExactPoints il punto esatto KPoint2
				if ( mapInexactPoints.find(seg.start()) == mapInexactPoints.end())
				{
					//seg.start() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_start(seg.start());
					KPoint2	exactPoint_start = seg.start();
					//Creo punto inesatto
					Point2 inexactPoint_start( CGAL::to_double(seg.start().x()), CGAL::to_double(seg.start().y()) );
					//std::cout << "S: " << CGAL::to_double(seg.start().x()) << " , " << CGAL::to_double(seg.start().y()) << "\n";
					//Creo il Vertex_handle
					Vertex_handle vStart = cdt.insert(inexactPoint_start);
					//std::cout << "A";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexact(seg.start(), vStart));
					//std::cout << "B";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExact(vStart, exactPoint_start));
					//std::cout << "C";
				}
				if ( mapInexactPoints.find(seg.end()) == mapInexactPoints.end())
				{
					//seg.end() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_end(seg.end());
					KPoint2	exactPoint_end = seg.end();
					//Creo punto inesatto
					Point2 inexactPoint_end( CGAL::to_double(seg.end().x()), CGAL::to_double(seg.end().y()) );
					//std::cout << "E: " << CGAL::to_double(seg.end().x()) << " , " << CGAL::to_double(seg.end().y()) << "\n";;
					//Creo il Vertex_handle
					Vertex_handle vEnd = cdt.insert(inexactPoint_end);
					//std::cout << "D";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexact(seg.end(), vEnd));
					//std::cout << "F";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExact(vEnd, exactPoint_end));
					//std::cout << "G";
				}

				#ifdef CUTTERMESHER_TEST_DEBUG
				std::cout << "*******************" << cdt.is_valid() << "********************" << std::endl;
				std::cout << "Generated mesh: faceBuilder_inexact: number of vertices: " << cdt.number_of_vertices()	<< std::endl;
				std::cout << "Generated mesh: faceBuilder_inexact: number of faces: " << cdt.number_of_faces() << std::endl;
				std::cout << "-----------------------------" << std::endl;
				#endif
			}
			else
			{
				isValidCDT = false;
				std::cout << "WARNING: faceBuilder_inexact: tried to insert same points in CutterMesher cdt.insert call!" << std::endl;
			}
		}
	}

	//TODO: in questo punto si potrebbero inserire altri punti casuali e renderli constraint in modo da non avere
	// una triangolazione con triangoli stretti e lunghissimi

	//STEP 2
	//Ciclo una seconda volta tutte le facce trovate nell'arrangement e inserisco il constraint sul CDT referenziando i punti esatti
	//e prendendo i Vertex_handle memorizzati in mapInexactPoints
	//NOTE: il contenuto interno dello step 2 potrebbe anche essere eseguto subito dopo i due if interni al ciclo precente in STEP 1
	if (isValidCDT)
	{
		for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
		{
			//Estraggo dal vettore FaceHoleInfo
			FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

			//Aggiungo i segmenti per i constraint della mesh
			//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
			std::list<Segment_2>::iterator faceSegsIter;
			for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
			{
				//std::cout << "*";
				Segment_2 seg = *faceSegsIter;
				if (seg.start() != seg.end())
				{

					MapInexactPoints::iterator foundVertexHandleStart;
					if ( ( foundVertexHandleStart = mapInexactPoints.find(seg.start())) == mapInexactPoints.end())
					{
						//Something went very bad
						std::cout << "ERROR: faceBuilder_inexact: Exact point start not found in mapInexactPoints!!" << std::endl;
						break;
					}

					MapInexactPoints::iterator foundVertexHandleEnd;
					if ( ( foundVertexHandleEnd = mapInexactPoints.find(seg.end())) == mapInexactPoints.end())
					{
						std::cout << "ERROR: faceBuilder_inexact: Exact point end not found in mapInexactPoints!!" << std::endl;
						break;
					}

					//Inserisco il constraint nel CDT
					//if (((float)CGAL::to_double(foundVertexHandleStart->second->point().x()) != (float)CGAL::to_double(foundVertexHandleEnd->second->point().x())) ||
					//		((float)CGAL::to_double(foundVertexHandleStart->second->point().y()) != (float)CGAL::to_double(foundVertexHandleEnd->second->point().y())) )
					if (foundVertexHandleStart->second->point() != foundVertexHandleEnd->second->point()) {
						try {
								cdt.insert_constraint(foundVertexHandleStart->second, foundVertexHandleEnd->second);
						}
						catch (CGAL::Assertion_exception e) {
							isValidCDT = false;
							std::cout << "WARNING: faceBuilder_inexact: CGAL::Assertion_exception Building CDT!" << std::endl;
						}
					} else {
						isValidCDT = false;
						std::cout << "WARNING: faceBuilder_inexact: tried to insert same points in CutterMesher cdt.insert_constraint call!" << std::endl;
					}

				}
			}
		}
	}

	//Genero la mesh
	if (isValidCDT)
	{
		#ifdef CUTTERMESHER_LOG
		std::cout << "Start for generating mesh faceBuilder_inexact: " << std::endl;
		std::cout << "Generated mesh: number of vertices: faceBuilder_inexact: " << cdt.number_of_vertices()	<< std::endl;
		std::cout << "Generated mesh: number of faces: faceBuilder_inexact: " << cdt.number_of_faces() << std::endl;
		#endif

		#ifdef CUTTERMESHER_LOG
			int numberTrianglesInPlanarMesh = 0; //Serve solo per debug per tenere traccia dei triangoli aggiunti nella faccia
		#endif

		std::list<Point_2>  exactBaryncenters;
		std::list<Triangle> planeMesh;
		//Ciclo le facce della mesh e creo i triangoli sul piano e li aggiungo in meshOutput
		for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
		{
			Face face = *fit;

			Point point3d[3];
			KPoint2 kpoint2[3];
			for (unsigned int vertexIdx = 0; vertexIdx < 3; vertexIdx++)
			{
				//Recupero il punto esatto corrispondenti alla frontiera della mesh, se c'e'
				MapExactPoints::iterator foundPair;
				if ( (foundPair = mapExactPoints.find(face.vertex(vertexIdx))) != mapExactPoints.end())
				{
					kpoint2[vertexIdx] = foundPair->second;
				}
				else
				{
					std::cout << "ERROR: faceBuilder_inexact: vertex not found in mapExactPoints!!" << std::endl;
				}

				//proietto il punto 2d sul piano 3d (Point kernel esatto)
				point3d[vertexIdx] = plane.to_3d(kpoint2[vertexIdx]);

			}

			Triangle triangle(point3d[0], point3d[1], point3d[2]);
			planeMesh.push_back(triangle);

			Point_2 exactBarycenter((kpoint2[0].x() + kpoint2[1].x() +kpoint2[2].x())/3.0, (kpoint2[0].y() + kpoint2[1].y() +kpoint2[2].y())/3.0);
			exactBaryncenters.push_back(exactBarycenter);
		}

		//Locate batch query points
		segmentsArrangment.locatePoints(exactBaryncenters);

		std::list<Triangle>::iterator triangleIterator;
		std::list<Point_2>::iterator  exactBarycentersIterator;
		for (triangleIterator = planeMesh.begin(), exactBarycentersIterator = exactBaryncenters.begin();
				(triangleIterator != planeMesh.end()) && (exactBarycentersIterator != exactBaryncenters.end());
				++triangleIterator, ++exactBarycentersIterator)
		{

			Triangle triangle = *triangleIterator;
			Point_2 exactBarycenter = *exactBarycentersIterator;

				if (segmentsArrangment.isInArrangment(exactBarycenter)) {

					//Aggiungo il triangolo alla mesh
					meshOutput.push_back(triangle);
					//Add triangle info for cut type
					addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, triangle, CUTTERMESHER_TRIANGLE_IN_CUT_PLANE);

					#ifdef CUTTERMESHER_LOG
						numberTrianglesInPlanarMesh++;
					#endif
				}

		}

		#ifdef CUTTERMESHER_LOG
		std::cout << "Added faces: faceBuilder_inexact: " << numberTrianglesInPlanarMesh << std::endl;
		#endif

	}

	return isValidCDT;
}


bool faceBuilder_exact(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo)
{
	//Recupero allFacesHolesVector dal SegmentsArrangement
	std::vector<FaceHoleInfo> allFacesHoles = segmentsArrangment.getAllFacesHolesVectorInObject();

	//Costruisco la mesh 2d e la proietto sul piano 3d e aggiunto i triangoli ad meshOutput

	//TODO:
	//Dal seed che appartiene ad una faccia (non ad un hole) e convertito in 3d sul piano,
	// se quel seed e' interno ad una superficie completamente chiusa rappresentata da meshInput
	// allora chiudo la superficie con la mesh calcolata di seguito

	//CDTEX Serve per la creazione della mesh triangoli, aggiungendo i segmenti delle facce
	CDTEX cdt;
	bool isValidCDT = true;
	//Lista che memorizza i seeds della mesh (punti in cui l'area non deve essere meshata)
	//Si puo' usare solo se viene usata la triangolazione Delaunay, sostituito dal metodo isInArrangment di SegmentsArrangement
	//std::list<Point2EX> list_of_seeds;

	//STEP 2
	//Ciclo e le facce trovate nell'arrangement e inserisco il constraint sul CDTEX
	for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

		//Aggiungo i segmenti per i constraint della mesh
		//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
		std::list<Segment_2>::iterator faceSegsIter;
		for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
		{
			//std::cout << "*";
			Segment_2 seg = *faceSegsIter;
			if (seg.start() != seg.end())
			{

				Vertex_handleEX vStart = cdt.insert(Point2EX(seg.start()));
				Vertex_handleEX vEnd = cdt.insert(Point2EX(seg.end()));

				try {
						cdt.insert_constraint(vStart, vEnd);
				}
				catch (CGAL::Assertion_exception e) {
					isValidCDT = false;
					std::cout << "WARNING: faceBuilder_exact: CGAL::Assertion_exception Building CDT!" << std::endl;
				}

			}
			else
			{
				isValidCDT = false;
				std::cout << "WARNING: faceBuilder_exact: tried to insert same points in CutterMesher cdt.insert and cdt.insert_constraint call!" << std::endl;
			}
		}
	}

	//TODO: in questo punto si potrebbero inserire altri punti casuali e renderli constraint in modo da non avere
	// una triangolazione con triangoli stretti e lunghissimi

	//Genero la mesh
	if (isValidCDT)
	{
		#ifdef CUTTERMESHER_LOG
		std::cout << "Start for generating mesh faceBuilder_exact" << std::endl;
		std::cout << "Generated mesh: faceBuilder_exact: number of vertices: " << cdt.number_of_vertices()	<< std::endl;
		std::cout << "Generated mesh: faceBuilder_exact: number of faces: " << cdt.number_of_faces() << std::endl;
		#endif

		#ifdef CUTTERMESHER_LOG
			int numberTrianglesInPlanarMesh = 0; //Serve solo per debug per tenere traccia dei triangoli aggiunti nella faccia
		#endif

		std::list<Point_2>  exactBaryncenters;
		std::list<Triangle> planeMesh;
		//Ciclo le facce della mesh e creo i triangoli sul piano e li aggiungo in meshOutput
		for (CDTEX::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
		{
			FaceEX face = *fit;

			Point point3d[3];
			KPoint2 kpoint2[3];
			for (unsigned int vertexIdx = 0; vertexIdx < 3; vertexIdx++)
			{
				Vertex_handleEX vh = face.vertex(vertexIdx);
				kpoint2[vertexIdx] = vh->point();
				//proietto il punto 2d sul piano 3d (Point kernel esatto)
				point3d[vertexIdx] = plane.to_3d(kpoint2[vertexIdx]);
			}

			Triangle triangle(point3d[0], point3d[1], point3d[2]);
			planeMesh.push_back(triangle);

			Point_2 exactBarycenter((kpoint2[0].x() + kpoint2[1].x() +kpoint2[2].x())/3.0, (kpoint2[0].y() + kpoint2[1].y() +kpoint2[2].y())/3.0);
			exactBaryncenters.push_back(exactBarycenter);
		}

		//Locate batch query points
		segmentsArrangment.locatePoints(exactBaryncenters);

		std::list<Triangle>::iterator triangleIterator;
		std::list<Point_2>::iterator  exactBarycentersIterator;
		for (triangleIterator = planeMesh.begin(), exactBarycentersIterator = exactBaryncenters.begin();
				(triangleIterator != planeMesh.end()) && (exactBarycentersIterator != exactBaryncenters.end());
				++triangleIterator, ++exactBarycentersIterator)
		{

			Triangle triangle = *triangleIterator;
			Point_2 exactBarycenter = *exactBarycentersIterator;

				if (segmentsArrangment.isInArrangment(exactBarycenter)) {

					//Aggiungo il triangolo alla mesh
					meshOutput.push_back(triangle);
					//Add triangle info for cut type
					addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, triangle, CUTTERMESHER_TRIANGLE_IN_CUT_PLANE);

					#ifdef CUTTERMESHER_LOG
						numberTrianglesInPlanarMesh++;
					#endif
				}

		}

		#ifdef CUTTERMESHER_LOG
		std::cout << "Added faces: faceBuilder_exact: " << numberTrianglesInPlanarMesh << std::endl;
		#endif

	}

	return isValidCDT;
}


bool faceBuilder_Delaunay(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo, double bCriteria, double sCriteria)
{
	//Recupero allFacesHolesVector dal SegmentsArrangement
	std::vector<FaceHoleInfo> allFacesHoles = segmentsArrangment.getAllFacesHolesVectorInObject();

	//Costruisco la mesh 2d e la proietto sul piano 3d e aggiunto i triangoli ad meshOutput

	//TODO:
	//Dal seed che appartiene ad una faccia (non ad un hole) e convertito in 3d sul piano,
	// se quel seed e' interno ad una superficie completamente chiusa rappresentata da meshInput
	// allora chiudo la superficie con la mesh calcolata di seguito

	//Usando la precisione CGAL::Simple_cartesian<double> in alcuni casi si ha un errore:
	//terminate called after throwing an instance of 'CGAL::Assertion_exception'
	//  what():  CGAL ERROR: assertion violation!
	//Expr: n == zone.fh
	//File: CGAL-3.9/include/CGAL/Mesh_2/Refine_edges.h
	//Line: 431
	// Usando Exact_predicates_inexact_constructions_kernel l'errore non si presenta,
	//  si presenta solo nel caso ci siano facce triangolari in FaceHoleInfo adiacenti ad altre facce
	//  e con forme che non possono rispettare la triangolazione di Delaunay

	//CDTDY Serve per la creazione della mesh triangoli, aggiungendo i segmenti delle facce
	CDTDY cdt;
	bool isValidCDT = true;
	//Lista che memorizza i seeds della mesh (punti in cui l'area non deve essere meshata)
	//Si puo' usare solo se viene usata la triangolazione Delaunay, sostituito dal metodo isInArrangment di SegmentsArrangement
	std::list<Point2DY> list_of_seeds;

	//Mappa che mappa i punti non esatti in esatti, per la frontiera della mesh
	// dato che la mesh usa un kernel inesatto, ci sarebbe un errore sui punti della frontiera
	MapExactPointsDY mapExactPoints;

	//Mappa che mappa Point_2 in Vertex_handleDY, per non creare piu' volte i stessi punti inesatti della mesh,
	//serve solo quando dai segmenti dell'arrangment si inseriscono i constraint per la mesh,
	//non viene utilizzata quando dalla mesh si ricreano i punti 3d
	MapInexactPointsDY mapInexactPoints;

	//STEP 1
	//Ciclo una prima volta tutte le facce trovate nell'arrangement e creo solo una volta i punti inesatti e li memorizzo in
	//mapExactPoints e mapInexactPoints mappando i punti esatti
	for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

		//Aggiungo i segmenti per i constraint della mesh
		//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
		std::list<Segment_2>::iterator faceSegsIter;
		for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
		{
			//std::cout << "+";

			Segment_2 seg = *faceSegsIter;
			if (seg.start() != seg.end())
			{
				
				#ifdef CUTTERMESHER_TEST_DEBUG
				std::cout << "[" <<seg.start().x() << ", " << seg.start().y() << " ; ";
				std::cout << seg.end().x() << ", " << seg.end().y() << "] " << std::endl;

				ListCSegment2d listasegtest2;
				CSegment2d seg2d;
				seg2d.startX =  CGAL::to_double(seg.start().x());
				seg2d.startY =  CGAL::to_double(seg.start().y());
				seg2d.endX =  CGAL::to_double(seg.end().x());
				seg2d.endY =  CGAL::to_double(seg.end().y());
				listasegtest2.push_back(seg2d);
				vettorelisteseg.push_back(listasegtest2);
				#endif

				//controllo se i punti seg.start() o seg.end() sono in mapInexactPoints
				// se non ci sono aggiunto alla mappa mapInexactPoints il Vertex_handleDY creato
				// e alla mappa mapExactPoints il punto esatto KPoint2
				if ( mapInexactPoints.find(seg.start()) == mapInexactPoints.end())
				{
					//seg.start() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_start(seg.start());
					KPoint2	exactPoint_start = seg.start();
					//Creo punto inesatto
					Point2DY inexactPoint_start( CGAL::to_double(seg.start().x()), CGAL::to_double(seg.start().y()) );
					//std::cout << "S: " << CGAL::to_double(seg.start().x()) << " , " << CGAL::to_double(seg.start().y()) << "\n";
					//Creo il Vertex_handleDY
					Vertex_handleDY vStart = cdt.insert(inexactPoint_start);
					//std::cout << "A";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexactDY(seg.start(), vStart));
					//std::cout << "B";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExactDY(vStart, exactPoint_start));
					//std::cout << "C";
				}
				if ( mapInexactPoints.find(seg.end()) == mapInexactPoints.end())
				{
					//seg.end() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_end(seg.end());
					KPoint2	exactPoint_end = seg.end();
					//Creo punto inesatto
					Point2DY inexactPoint_end( CGAL::to_double(seg.end().x()), CGAL::to_double(seg.end().y()) );
					//std::cout << "E: " << CGAL::to_double(seg.end().x()) << " , " << CGAL::to_double(seg.end().y()) << "\n";;
					//Creo il Vertex_handleDY
					Vertex_handleDY vEnd = cdt.insert(inexactPoint_end);
					//std::cout << "D";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexactDY(seg.end(), vEnd));
					//std::cout << "F";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExactDY(vEnd, exactPoint_end));
					//std::cout << "G";
				}

				#ifdef CUTTERMESHER_TEST_DEBUG
				std::cout << "*******************" << cdt.is_valid() << "********************" << std::endl;
				std::cout << "Generated mesh: faceBuilder_Delaunay: number of vertices: " << cdt.number_of_vertices()	<< std::endl;
				std::cout << "Generated mesh: faceBuilder_Delaunay: number of faces: " << cdt.number_of_faces() << std::endl;
				std::cout << "-----------------------------" << std::endl;
				#endif
			}
			else
			{
				isValidCDT = false;
				std::cout << "WARNING: faceBuilder_Delaunay: tried to insert same points in CutterMesher cdt.insert call!" << std::endl;
			}
		}
	}

	//STEP 2
	//Ciclo una seconda volta tutte le facce trovate nell'arrangement e inserisco il constraint sul CDT referenziando i punti esatti
	//e prendendo i Vertex_handleDY memorizzati in mapInexactPoints
	//NOTE: il contenuto interno dello step 2 potrebbe anche essere eseguto subito dopo i due if interni al ciclo precente in STEP 1
	if (isValidCDT)
	{
		for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
		{
			//Estraggo dal vettore FaceHoleInfo
			FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

			//Aggiungo i segmenti per i constraint della mesh
			//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
			std::list<Segment_2>::iterator faceSegsIter;
			for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
			{
				//std::cout << "*";
				Segment_2 seg = *faceSegsIter;
				if (seg.start() != seg.end())
				{

					MapInexactPointsDY::iterator foundVertexHandleStart;
					if ( ( foundVertexHandleStart = mapInexactPoints.find(seg.start())) == mapInexactPoints.end())
					{
						//Something went very bad
						std::cout << "ERROR: faceBuilder_Delaunay: Exact point start not found in mapInexactPoints!!" << std::endl;
						break;
					}

					MapInexactPointsDY::iterator foundVertexHandleEnd;
					if ( ( foundVertexHandleEnd = mapInexactPoints.find(seg.end())) == mapInexactPoints.end())
					{
						std::cout << "ERROR: faceBuilder_Delaunay: Exact point end not found in mapInexactPoints!!" << std::endl;
						break;
					}

					//Inserisco il constraint nel CDT
					//if (((float)CGAL::to_double(foundVertexHandleStart->second->point().x()) != (float)CGAL::to_double(foundVertexHandleEnd->second->point().x())) ||
					//		((float)CGAL::to_double(foundVertexHandleStart->second->point().y()) != (float)CGAL::to_double(foundVertexHandleEnd->second->point().y())) )
					if (foundVertexHandleStart->second->point() != foundVertexHandleEnd->second->point()) {
						try {
								cdt.insert_constraint(foundVertexHandleStart->second, foundVertexHandleEnd->second);
						}
						catch (CGAL::Assertion_exception e) {
							isValidCDT = false;
							std::cout << "WARNING: faceBuilder_Delaunay: CGAL::Assertion_exception Building CDT!" << std::endl;
						}
					} else {
						isValidCDT = false;
						std::cout << "WARNING: faceBuilder_Delaunay: tried to insert same points in CutterMesher cdt.insert_constraint call!" << std::endl;
					}

				}
			}
		}
	}

	//Genero la mesh
	if (isValidCDT)
	{
		//Genero la Delaunay mesh
		//For the Criteria parameters see the page:
		// http://doc.cgal.org/latest/Mesh_2/classCGAL_1_1Delaunay__mesh__size__criteria__2.html
		// the parameter default values are bCriteria = 0.125 and sCriteria = 0.0
		//std::cout << "bCriteria: " << bCriteria << ", sCriteria: " << sCriteria << std::endl;
		CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria(bCriteria, sCriteria));
		//CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria());
		//CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria(0.1, 0.3));
		//CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria(2, 2));

		#ifdef CUTTERMESHER_LOG
		std::cout << "Start for generating mesh faceBuilder_Delaunay: " << std::endl;
		std::cout << "Generated mesh: number of vertices: faceBuilder_Delaunay: " << cdt.number_of_vertices()	<< std::endl;
		std::cout << "Generated mesh: number of faces: faceBuilder_Delaunay: " << cdt.number_of_faces() << std::endl;
		#endif

		#ifdef CUTTERMESHER_LOG
			int numberTrianglesInPlanarMesh = 0; //Serve solo per debug per tenere traccia dei triangoli aggiunti nella faccia
		#endif

		std::list<Point_2>  exactBaryncenters;
		std::list<Triangle> planeMesh;
		//Ciclo le facce della mesh e creo i triangoli sul piano e li aggiungo in meshOutput
		for (CDTDY::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
		{
			FaceDY face = *fit;

			Point point3d[3];
			KPoint2 kpoint2[3];
			for (unsigned int vertexIdx = 0; vertexIdx < 3; vertexIdx++)
			{
				//Recupero il punto esatto corrispondenti alla frontiera della mesh, se c'e'
				MapExactPointsDY::iterator foundPair;
				if ( (foundPair = mapExactPoints.find(face.vertex(vertexIdx))) != mapExactPoints.end())
				{
					kpoint2[vertexIdx] = foundPair->second;
				}
				else
				{
					//face.vertex(vertexIdx) non e' stato trovato in mapExactPoints:
					// pertanto creo un punto KPoint2 da face.vertex(vertexIdx)
					// e aggiungo una nuova entry in mapExactPoints in modo da poterla riutilizzare, se viene incontrato di nuovo quel punto.
					//Per risolvere il problema dei punti creati sulla frontiera constraint:
					// proiettare i punti creati in piu sulla frontiera sul segmento del constraint, in modo da avere un punto
					// che giace sul segmento e non avere un errore di chiusura tra la faccia di taglio e bordi,
					// usando Constrained_triangulation_plus_2<Tr> per ottenere i segmenti constraint da cdt

					bool pointOnConstrain = false;
					//trovo il segmento constrain se il vertice cade su un segmento constraint,
					// e associo al punto face.vertex(vertexIdx), il punto esatto proiettato sul segmento esatto
					if (cdt.are_there_incident_constraints(face.vertex(vertexIdx)))
					{
						std::list<EdgeDY> constraintsList;
						cdt.incident_constraints(face.vertex(vertexIdx), std::back_inserter(constraintsList));
						if (constraintsList.size() > 0)
						{
							std::list<EdgeDY>::iterator constraintEdgeIter;
							for (constraintEdgeIter = constraintsList.begin(); constraintEdgeIter != constraintsList.end(); ++constraintEdgeIter)
							{
								EdgeDY constrEdge = *constraintEdgeIter;
								if (!cdt.is_constrained(constrEdge))
								{
									std::cout << "WARNING: faceBuilder_Delaunay: not is_constrained!" << std::endl;
								}
								else if ( cdt.are_there_incident_constraints( constrEdge.first->vertex( (constrEdge.second+1) %3)  ) &&
										cdt.are_there_incident_constraints(  constrEdge.first->vertex( (constrEdge.second+2) %3)   ) )
								{

									//int num1 = cdt.number_of_enclosing_constraints( constrEdge.first->vertex( (constrEdge.second+1) %3),
									//		constrEdge.first->vertex( (constrEdge.second+2) %3));
									//int num2 = cdt.number_of_enclosing_constraints( constrEdge.first->vertex( (constrEdge.second+2) %3),
									//		constrEdge.first->vertex( (constrEdge.second+1) %3));

									ContextDY context = cdt.context( constrEdge.first->vertex( (constrEdge.second+1) %3),
											constrEdge.first->vertex( (constrEdge.second+2) %3));

									Vertex_handleDY vStartConstr = *(context.vertices_begin());
									Vertex_handleDY vEndConstr;
									Vertices_in_constraint_iteratorDY iterVertCont;
									for (iterVertCont = context.vertices_begin(); iterVertCont != context.vertices_end(); ++iterVertCont)
									{
										vEndConstr = *iterVertCont;
									}

									MapExactPointsDY::iterator foundPairStart = mapExactPoints.find(vStartConstr);
									MapExactPointsDY::iterator foundPairEnd = mapExactPoints.find(vEndConstr);
									if ( (foundPairStart != mapExactPoints.end()) && (foundPairEnd != mapExactPoints.end()) )
									{
										//Costruisco la linea esatta con i vertici del constraint trovato
										Line_2 lineConstraint( foundPairStart->second, foundPairEnd->second);
										//Proietto il punto inesatto sulla linea, trovando il corrispondente punto esatto
										kpoint2[vertexIdx] = lineConstraint.projection(KPoint2(face.vertex(vertexIdx)->point().x(), face.vertex(vertexIdx)->point().y()));
										//Aggiungo alla mappa mapExactPoints
										mapExactPoints.insert(PairPointsInexactExactDY(face.vertex(vertexIdx), kpoint2[vertexIdx]));

										pointOnConstrain = true;
										break; //ciclo for on constraintEdgeIter
									}
									else
									{
										std::cout << "WARNING: faceBuilder_Delaunay: don't find constrain points!" << std::endl;
									}
								}
								else
								{
									std::cout << "WARNING: faceBuilder_Delaunay: NOT are_there_incident_constraints!" << std::endl;
								}
							}
						}
					}

					if (!pointOnConstrain)
					{
						//Punto interno alla faccia, che non cade su un constraint:
						// genero semplicemente un punto esatto corrispondente e lo aggiungo alla mappa mapExactPoints
						kpoint2[vertexIdx] = KPoint2(face.vertex(vertexIdx)->point().x(), face.vertex(vertexIdx)->point().y());
						//Aggiungo alla mappa mapExactPoints
						mapExactPoints.insert(PairPointsInexactExactDY(face.vertex(vertexIdx), kpoint2[vertexIdx]));
					}

				}

				//proietto il punto 2d sul piano 3d (Point kernel esatto)
				point3d[vertexIdx] = plane.to_3d(kpoint2[vertexIdx]);

			}

			Triangle triangle(point3d[0], point3d[1], point3d[2]);
			planeMesh.push_back(triangle);

			Point_2 exactBarycenter((kpoint2[0].x() + kpoint2[1].x() +kpoint2[2].x())/3.0, (kpoint2[0].y() + kpoint2[1].y() +kpoint2[2].y())/3.0);
			exactBaryncenters.push_back(exactBarycenter);
		}

		//Locate batch query points
		segmentsArrangment.locatePoints(exactBaryncenters);

		std::list<Triangle>::iterator triangleIterator;
		std::list<Point_2>::iterator  exactBarycentersIterator;
		for (triangleIterator = planeMesh.begin(), exactBarycentersIterator = exactBaryncenters.begin();
				(triangleIterator != planeMesh.end()) && (exactBarycentersIterator != exactBaryncenters.end());
				++triangleIterator, ++exactBarycentersIterator)
		{

			Triangle triangle = *triangleIterator;
			Point_2 exactBarycenter = *exactBarycentersIterator;

				if (segmentsArrangment.isInArrangment(exactBarycenter)) {

					//Aggiungo il triangolo alla mesh
					meshOutput.push_back(triangle);
					//Add triangle info for cut type
					addInTrianglesInfoOutput(trianglesInfoOutput, mapIdTriangleInfo, triangle, CUTTERMESHER_TRIANGLE_IN_CUT_PLANE);

					#ifdef CUTTERMESHER_LOG
						numberTrianglesInPlanarMesh++;
					#endif
				}

		}

		#ifdef CUTTERMESHER_LOG
		std::cout << "Added faces: faceBuilder_Delaunay: " << numberTrianglesInPlanarMesh << std::endl;
		#endif

	}

	return isValidCDT;
}


//Deprecated: it doesn't work. Inserisce errore sui punti interni della mesh del piano tagliato
//TODO: si potrebbe proiettare i punti creati in piu sulla frontiera sul segmento del constraint, in modo da avere un punto
// che giace sul segmento e non avere un errore di chiusura tra la faccia di taglio e bordi,
// usando Constrained_triangulation_plus_2<Tr> per ottenere i segmenti constraint da cdt
// Actually: it is what the new faceBuilder_Delaunay function does!
std::list<Triangle> cutMesh_UsingDelaunayMesh(std::list<Triangle> &meshInput, Plane plane) {

	CGAL::Timer timer;
	timer.start();

	std::cout << "meshInput total triangles: " << meshInput.size() << std::endl;

	std::list<Triangle> meshOutput;
	std::list<Triangle> intersectTriangles;
	std::list<Segment> intersectSegments;

	//Aggiungo nella mesh solo i triangoli che hanno tutti i vertici con distanza dal piano positiva
	std::cout << "Adding Triangles in positive plane and intersectTriangles" << std::endl;
	std::list<Triangle>::iterator triangleIter;
	for(triangleIter=meshInput.begin(); triangleIter!= meshInput.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;
		bool isTrianglePointDistanceSmaller[3];
		isTrianglePointDistanceSmaller[0] = isSignedDistancePositive(plane, t.vertex(0));
		isTrianglePointDistanceSmaller[1] = isSignedDistancePositive(plane, t.vertex(1));
		isTrianglePointDistanceSmaller[2] = isSignedDistancePositive(plane, t.vertex(2));

		if (isTrianglePointDistanceSmaller[0] &&
			isTrianglePointDistanceSmaller[1] &&
			isTrianglePointDistanceSmaller[2] )
		{
			//Il triangolo e' tutto sopra il piano quindi lo aggiungo alla mesh di output
			meshOutput.push_back(t);

		} else if (!isTrianglePointDistanceSmaller[0] &&
				!isTrianglePointDistanceSmaller[1] &&
				!isTrianglePointDistanceSmaller[2])
		{
			//Il triangolo e' tutto sotto il piano quindi lo scarto
		} else {
			//Il triangolo e' intersecato dal piano quindi aggiungo il triangolo a intersectTriangles
			intersectTriangles.push_back(t);
		}

	}

	std::cout << "Added Triangles in meshOutput: " << meshOutput.size() << std::endl;
	std::cout << "Added Triangles in intersectTriangles: " << intersectTriangles.size() << std::endl;

	std::cout << "Building tree" << std::endl;
	Tree tree(intersectTriangles.begin(), intersectTriangles.end());

	std::list<Object_and_primitive_id> intersections;
	std::cout << "Start tree.all_intersections" << std::endl;
	tree.all_intersections(plane, std::back_inserter(intersections));
	std::cout << "End tree.all_intersections: total: " << intersections.size() << " intersections" << std::endl;

	std::list<Object_and_primitive_id>::iterator intersIter;
	std::cout << "Start compute for intersections..." << std::endl;
	for(intersIter=intersections.begin(); intersIter!= intersections.end(); ++intersIter)
	{

		Object_and_primitive_id op = *intersIter;

		//Get intersection primitive
		Primitive id = op.second;
		Triangle t = id.datum();

		//Get intersection points
		CGAL::Object object = op.first;
		Segment segment;
		Point point;
		Triangle triangle;
		if(CGAL::assign(segment,object))
		{
			//Aggiungo i segmenti per il bordo
			intersectSegments.push_back(segment);
		}
		else if(CGAL::assign(point,object))
		{
			//std::cout << "WARNING: intersection object is a Point" << std::endl;
			//Se l'oggetto di intersezione e' un punto allora deve essere il vertice di un triangolo che devo
			// aggiungere in meshOutput altrimenti viene escluso dalla striscia di intersezione
			meshOutput.push_back(t);
			std::cout << "INFO: intersection object is a point" << std::endl;
			continue;
		}
		else if(CGAL::assign(triangle,object))
		{
			std::cout << "WARNING: intersection object is a triangle" << std::endl;
			continue;
		}
		else
		{
			std::cout << "WARNING: intersection object is NOT segment or point" << std::endl;
			continue;
		}

		//Ricostruzione striscia di intersezione
		int numPositivePoints = 0;
		int indexVertexNegativePoint = -1;
		Point positivePoints[3];

		for (int i = 0;  i < 3; ++i)
		{
			Point triangleVertex = t.vertex(i);
			//std::cout << triangleVertex.id() <<" " << t.vertex(i).id() << " " << (triangleVertex == t.vertex(i)) << "\n";
			if (isSignedDistancePositive(plane, triangleVertex))
			{
				positivePoints[numPositivePoints] = triangleVertex;
				numPositivePoints++;
			} else {
				indexVertexNegativePoint = i;
			}
		}

		//std::cout << "numPositivePoints: " <<  numPositivePoints << std::endl;
		if (numPositivePoints == 3) {

			std::cout << "WARNING: numPositivePoints: " <<  numPositivePoints << std::endl;
			continue;

		} else if (numPositivePoints == 1) {

			meshOutput.push_back(Triangle( positivePoints[0],segment.start(),segment.end()));

		} else if (numPositivePoints == 2) {

			//Se sono qui ci deve essere un solo vertice nella parte
			// negativa del piano corrispondente all'indice indexVertexNegativePoint
			Point vertexNegative = t.vertex(indexVertexNegativePoint);
			Segment triangleSide(vertexNegative, positivePoints[0]);
			FT distance1 = squared_distance(triangleSide,  segment.start());
			FT distance2 = squared_distance(triangleSide,  segment.end());
			//std::cout << distance1 << "   " <<  distance2 << std::endl;

			//if (triangleSide.has_on(segment.start())) { //Non funziona a causa dell'errore float
			if ( distance1 < distance2) {

				//
				//    1---2
				//    | \ |
				//    |  \|
				//    3---4
				//
				meshOutput.push_back(Triangle( positivePoints[0], positivePoints[1], segment.start()));
				meshOutput.push_back(Triangle( positivePoints[1], segment.start(), segment.end()));

			//} else if (triangleSide.has_on(segment.end())) { //Non funziona a causa dell'errore float
			} else {

				meshOutput.push_back(Triangle( positivePoints[0], positivePoints[1], segment.end()));
				meshOutput.push_back(Triangle( positivePoints[1], segment.start(), segment.end()));

			}

		} //End if costruzione triangoli striscia di intersezione

	} //End for intersections


	//Costruisco la faccia tagliata

	//Vettore di segmenti 2d del contorno di intersezione mesh-piano
	std::vector<Segment_2> outlineSegements;

	#ifdef CUTTERMESHER_TEST_DEBUG
	std::vector<Point> punti3d_frontiera;
	ListCSegment2d listasegtest;
	ListSegment_2 listaSegs_2;
	#endif

	//Proietto i segmenti 3d in intersectSegments sul piano plane
	std::list<Segment>::iterator segmentIter;
	for (segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
	{
		Segment segment3d = *segmentIter;
		Point start3d = segment3d.start();
		Point end3d = segment3d.end();

		#ifdef CUTTERMESHER_TEST_DEBUG
		punti3d_frontiera.push_back(start3d);
		punti3d_frontiera.push_back(end3d);
		#endif

		//Proietto il punto 3d sul piano e ottengo un punto 2d
		KPoint2 start = plane.to_2d(start3d);
		KPoint2 end = plane.to_2d(end3d);

		//Se il segmento non e' degenere lo aggiungo ad outlineSegements
		//if ((start.x()!=end.x()) || (start.y()!=end.y()))
		if ( start!=end )
		{
			//Segment_2 segment2d(Point_2(start.x(), start.y()), Point_2(end.x(), end.y()));
			Segment_2 segment2d(start, end);
			outlineSegements.push_back(segment2d);

			#ifdef CUTTERMESHER_TEST_DEBUG
			CSegment2d seg2d;
			seg2d.startX =  CGAL::to_double(start.x());
			seg2d.startY =  CGAL::to_double(start.y());
			seg2d.endX =  CGAL::to_double(end.x());
			seg2d.endY =  CGAL::to_double(end.y());
			listasegtest.push_back(seg2d);
			listaSegs_2.push_back(segment2d);
			#endif

		}

	}

	#ifdef CUTTERMESHER_TEST_DEBUG
	vettorelisteseg.push_back(listasegtest);
	findIncongruencies(listaSegs_2);
	#endif

	//computo la gerarchia delle facce
	std::cout << "outlineSegements total: " << outlineSegements.size() << std::endl;
	std::cout << "Start for getFacesHolesHierarchy" << std::endl;
	SegmentsArrangement segmentsArrangment;
	std::vector<FaceHoleInfo> allFacesHoles = segmentsArrangment.getFacesHolesHierarchy(outlineSegements);
	std::cout << "End for getFacesHolesHierarchy: total:" << allFacesHoles.size() << " faces" << std::endl;

	if (allFacesHoles.size() > 0)
	{
		//Costruisco la mesh 2d e la proietto sul piano 3d e aggiunto i triangoli ad meshOutput

		//TODO:
		//Dal seed che appartiene ad una faccia (non ad un hole) e convertito in 3d sul piano,
		// se quel seed e' interno ad una superficie completamente chiusa rappresentata da meshInput
		// allora chiudo la superficie con la mesh calcolata di seguito

		//NOTE:
		//Per il calcolo della mesh posso anche usare la precisione CGAL::Simple_cartesian<double>
		// ma comunque proiettare i triangoli della mesh sul piano usando la precisione  CGAL::Exact_predicates_exact_constructions_kernel

		//CDT Serve per la creazione della mesh triangoli, aggiungendo i segmenti delle facce
		CDT cdt;
		//Lista che memorizza i seeds della mesh (punti in cui l'area non deve essere meshata)
		//Si puo' usare solo se viene usata la triangolazione Delaunay, sostituito dal metodo isInArrangment di SegmentsArrangement
		//std::list<Point2> list_of_seeds;

		//Mappa che mappa i punti non esatti in esatti, per la frontiera della mesh
		// dato che la mesh usa un kernel inesatto, ci sarebbe un errore sui punti della frontiera
		MapExactPoints mapExactPoints;

		//Mappa che mappa Point_2 in Vertex_handle, per non creare piu' volte i stessi punti inesatti della mesh,
		//serve solo quando dai segmenti dell'arrangment si inseriscono i constraint per la mesh,
		//non viene utilizzata quando dalla mesh si ricreano i punti 3d
		MapInexactPoints mapInexactPoints;

		//STEP 1
		//Ciclo una prima volta tutte le facce trovate nell'arrangement e creo solo una volta i punti inesatti e li memorizzo in
		//mapExactPoints e mapInexactPoints mappando i punti esatti
		for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
		{
			//Estraggo dal vettore FaceHoleInfo
			FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

			//Aggiungo i segmenti per i constraint della mesh
			//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
			std::list<Segment_2>::iterator faceSegsIter;
			for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
			{
				//std::cout << "+";

				Segment_2 seg = *faceSegsIter;
				//controllo se i punti seg.start() o seg.end() sono in mapInexactPoints
				// se non ci sono aggiunto alla mappa mapInexactPoints il Vertex_handle creato
				// e alla mappa mapExactPoints il punto esatto KPoint2
				if ( mapInexactPoints.find(seg.start()) == mapInexactPoints.end())
				{
					//seg.start() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_start(seg.start());
					KPoint2	exactPoint_start = seg.start();
					//Creo punto inesatto
					Point2 inexactPoint_start( CGAL::to_double(seg.start().x()), CGAL::to_double(seg.start().y()) );
					//std::cout << "S: " << CGAL::to_double(seg.start().x()) << " , " << CGAL::to_double(seg.start().y()) << "\n";
					//Creo il Vertex_handle
					Vertex_handle vStart = cdt.insert(inexactPoint_start);
					//std::cout << "A";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexact(seg.start(), vStart));
					//std::cout << "B";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExact(vStart, exactPoint_start));
					//std::cout << "C";
				}
				if ( mapInexactPoints.find(seg.end()) == mapInexactPoints.end())
				{
					//seg.end() non e' presente nella mappa mapInexactPoints
					//Creo punto esatto
					//KPoint2	exactPoint_end(seg.end());
					KPoint2	exactPoint_end = seg.end();
					//Creo punto inesatto
					Point2 inexactPoint_end( CGAL::to_double(seg.end().x()), CGAL::to_double(seg.end().y()) );
					//std::cout << "E: " << CGAL::to_double(seg.end().x()) << " , " << CGAL::to_double(seg.end().y()) << "\n";;
					//Creo il Vertex_handle
					Vertex_handle vEnd =   cdt.insert(inexactPoint_end);
					//std::cout << "D";
					//Aggiungo alla mappa mapInexactPoints
					mapInexactPoints.insert(PairPointsExactInexact(seg.end(), vEnd));
					//std::cout << "F";
					//Aggiungo alla mappa mapExactPoints
					mapExactPoints.insert(PairPointsInexactExact(vEnd, exactPoint_end));
					//std::cout << "G";
				}
			}
		}

		//STEP 2
		//Ciclo una seconda volta tutte le facce trovate nell'arrangement e inserisco il constraint sul CDT referenziando i punti esatti
		//e prendendo i Vertex_handle memorizzati in mapInexactPoints
		//NOTE: il contenuto interno dello step 2 potrebbe anche essere eseguto subito dopo i due if interni al ciclo precente in STEP 1
		for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
		{
			//Estraggo dal vettore FaceHoleInfo
			FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

			//Aggiungo i segmenti per i constraint della mesh
			//std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
			std::list<Segment_2>::iterator faceSegsIter;
			for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
			{
				//std::cout << "*";
				Segment_2 seg = *faceSegsIter;

				MapInexactPoints::iterator foundVertexHandleStart;
				if ( ( foundVertexHandleStart = mapInexactPoints.find(seg.start())) == mapInexactPoints.end())
				{
					//Something went very bad
					std::cout << "ERROR: Exact point start not found in mapInexactPoints!!" << std::endl;
					break;
				}

				MapInexactPoints::iterator foundVertexHandleEnd;
				if ( ( foundVertexHandleEnd = mapInexactPoints.find(seg.end())) == mapInexactPoints.end())
				{
					std::cout << "ERROR: Exact point end not found in mapInexactPoints!!" << std::endl;
					break;
				}

				//Inserisco il constraint nel CDT
				cdt.insert_constraint(foundVertexHandleStart->second, foundVertexHandleEnd->second);

			}

			//Posizionamente dei punti seeds Delaunay
			//if (!faceInfo->isFace) {
			//	Point_2 seed = faceInfo->seed;
			//	list_of_seeds.push_back(Point2(CGAL::to_double(seed.x()), CGAL::to_double(seed.y())));
			//}

		}

				//Il ciclo for seguente potrebbe inserire un errore a causa della creazione piu' volte dei stessi punti inesatti;
				//sostituito dal doppio ciclo precendente sulle facce dell'arrangment e sui segmenti
				/*
				//Ciclo le facce trovate nell'arrangement e aggiungo i segmenti e i seeds per generare la mesh CDT
				for (unsigned int faceIdx = 0; faceIdx < allFacesHoles.size(); faceIdx++)
				{
					//Estraggo dal vettore FaceHoleInfo
					FaceHoleInfo *faceInfo = &allFacesHoles.at(faceIdx);

					//Aggiungo i segmenti per i constraint della mesh
					std::multiset<Segment_2,classSegComp>::iterator faceSegsIter;
					for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
					{
						Segment_2 seg = *faceSegsIter;
						KPoint2	exactPoint_start(seg.start());
						Point2 inexactPoint_start( CGAL::to_double(seg.start().x()), CGAL::to_double(seg.start().y()) );
						KPoint2	exactPoint_end(seg.end());
						Point2 inexactPoint_end( CGAL::to_double(seg.end().x()), CGAL::to_double(seg.end().y()) );

						//Creazione mesh: aggiungo il segmento della faccia al cdt e al cdt constraint
						//Vertex_handle vStart = cdt.insert(Point2(CGAL::to_double(seg.start().x()), CGAL::to_double(seg.start().y()) ));
						//Vertex_handle vEnd =   cdt.insert(Point2(CGAL::to_double(seg.end().x()),   CGAL::to_double(seg.end().y()) ));
						Vertex_handle vStart = cdt.insert(inexactPoint_start);
						Vertex_handle vEnd =   cdt.insert(inexactPoint_end);
						cdt.insert_constraint(vStart, vEnd);

						//Memorizzo i punti nella mappa che mappa i punti non esatti in esatti, per la frontiera della mesh
						//mapExactPoints.insert(PairPointsInexactExact(inexactPoint_start ,exactPoint_start));
						//mapExactPoints.insert(PairPointsInexactExact(inexactPoint_end ,exactPoint_end));
						mapExactPoints.insert(PairPointsInexactExact(vStart ,exactPoint_start));
						mapExactPoints.insert(PairPointsInexactExact(vEnd ,exactPoint_end));

					}

					//Posizionamente dei punti seeds
					if (!faceInfo->isFace) {
						Point_2 seed = faceInfo->seed;
						list_of_seeds.push_back(Point2(CGAL::to_double(seed.x()), CGAL::to_double(seed.y())));
					}

				}
				std::cout << "mapExactPoints size: " << mapExactPoints.size()  << std::endl;
				*/

		//Genero la mesh
		//list_of_seeds.push_back(Point2(500.0, -350.0));
		std::cout << "Start for generating mesh" << std::endl;
		//Scommentare le linee di seguito per usare Delaunay mesh del piano tagliato
		//CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria());
		////CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria(0.1, 0.3));
		////CGAL::refine_Delaunay_mesh_2(cdt, list_of_seeds.begin(), list_of_seeds.end(), Criteria(2, 2));
		std::cout << "Generated mesh: number of vertices: " << cdt.number_of_vertices()	<< std::endl;
		std::cout << "Generated mesh: number of faces: " << cdt.number_of_faces() << std::endl;

		//Mappa che mappa tutti i vertex_handle non esatti in Point 3d esatti, gia' proiettati sul piano
		//Serve solo se la generazione della mesh ha creato punti interni che non sono punti delle facce originali di partenza
		// quindi utilizzato da delaunay mesh, dato che questa crea punti oltre i punti dei contraint originali di partenza
		MapExactPoints3d mapExactPoints3d;

#ifdef CUTTERMESHER_TEST_DEBUG
std::cout << "Test Cerco frontiera: intersectSegments.size: " << intersectSegments.size() << std::endl;
ListCSegment2d puntiFrontiera;
ListCSegment2d puntiInterni;
#endif

		//Ciclo le facce della mesh e creo i triangoli sul piano e li aggiungo in meshOutput
		for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
		{
			Face face = *fit;

			Point2 points[3];
			points[0] = face.vertex(0)->point();
			points[1] = face.vertex(1)->point();
			points[2] = face.vertex(2)->point();

			Point_2 baricenter((points[0].x() + points[1].x() +points[2].x())/3, (points[0].y() + points[1].y() +points[2].y())/3);

			if (segmentsArrangment.isInArrangment(baricenter)) {
			//Se si usano i seeds dal SegmentsArrangment e Delaunay mesh allora si può usare il metodo di seguito al posto di isInArrangment
			//if (face.is_in_domain()) {

#ifdef CUTTERMESHER_TEST_DEBUG
ListCSegment2d triangoliMesh;
CSegment2d segA;
segA.startX =  CGAL::to_double(points[0].x());
segA.startY =  CGAL::to_double(points[0].y());
segA.endX =  CGAL::to_double(points[1].x());
segA.endY =  CGAL::to_double(points[1].y());
triangoliMesh.push_back(segA);
CSegment2d segB;
segB.startX =  CGAL::to_double(points[1].x());
segB.startY =  CGAL::to_double(points[1].y());
segB.endX =  CGAL::to_double(points[2].x());
segB.endY =  CGAL::to_double(points[2].y());
triangoliMesh.push_back(segB);
CSegment2d segC;
segC.startX =  CGAL::to_double(points[2].x());
segC.startY =  CGAL::to_double(points[2].y());
segC.endX =  CGAL::to_double(points[0].x());
segC.endY =  CGAL::to_double(points[0].y());
triangoliMesh.push_back(segC);
vettorelisteseg.push_back(triangoliMesh);
#endif


				//Per tutti i vertici della face cerco se in mapExactPoints3d esiste gia' un punto 3d esatto,
				//se non esiste lo creo e lo inserisco
				for (unsigned int vertexIdx = 0; vertexIdx < 3; vertexIdx++)
				{
					if ( mapExactPoints3d.find(face.vertex(vertexIdx)) == mapExactPoints3d.end())
					{
						//face.vertex(vertexIdx) non e' presente nella mappa mapExactPoints3d pertanto lo creo e lo inserisco

						Point2 point2_vertex_handle = face.vertex(vertexIdx)->point();

						KPoint2 kpoint2;

						//Recupero il punto esatto corrispondenti alla frontiera della mesh, se c'e'
						MapExactPoints::iterator foundPair;

						if ( (foundPair = mapExactPoints.find(face.vertex(vertexIdx))) != mapExactPoints.end())
						{
							//std::cout << "*";
							kpoint2 = foundPair->second;
						}
						else
						{
							//Creo un punto esatto KPoint2
							kpoint2 = KPoint2(point2_vertex_handle.x(), point2_vertex_handle.y());
						}

						//proietto il punto 2d sul piano 3d (Point kernel esatto)
						Point point3d = plane.to_3d(kpoint2);

						//inserisco il punto trovato nella mappa
						mapExactPoints3d.insert(PairPointsInexactExact3d(face.vertex(vertexIdx), point3d));

#ifdef CUTTERMESHER_TEST_DEBUG
CSegment2d s1;
s1.startX =  CGAL::to_double(kpoint2.x()+0.01);
s1.startY =  CGAL::to_double(kpoint2.y()+0.01);
s1.endX =  CGAL::to_double(kpoint2.x()-0.01);
s1.endY =  CGAL::to_double(kpoint2.y()-0.01);
CSegment2d s2;
s2.startX =  CGAL::to_double(kpoint2.x()+0.01);
s2.startY =  CGAL::to_double(kpoint2.y()-0.01);
s2.endX =  CGAL::to_double(kpoint2.x()-0.01);
s2.endY =  CGAL::to_double(kpoint2.y()+0.01);
puntiFrontiera.push_back(s1);
puntiFrontiera.push_back(s2);
puntiInterni.push_back(s1);
puntiInterni.push_back(s2);
#endif

						//std::cout << "F";
					}
					//else
					//{
					//	std::cout << "X";
					//}
				}

				//In questo punto i punti 3d esatti devono essere presenti per forza nella mappa mapExactPoints3d,
				//li recupero e li uso per creare il triangolo
				//Questa volta non ho fatto due cicli for distinti (STEP 1 e 2) come nella creazione dei constraint del CDT,
				//giusto per utilizzare un diverso stile
				//TODO: potrei usare un array di MapExactPoints3d::iterator e un ciclo for per popolarlo
				MapExactPoints3d::iterator foundPoint3d_0;
				if ( ( foundPoint3d_0 = mapExactPoints3d.find(face.vertex(0))) == mapExactPoints3d.end())
				{
					//Something went very bad
					std::cout << "ERROR: Exact point 3d: 0 not found in mapExactPoints3d!!" << std::endl;
					break;
				}
				MapExactPoints3d::iterator foundPoint3d_1;
				if ( ( foundPoint3d_1 = mapExactPoints3d.find(face.vertex(1))) == mapExactPoints3d.end())
				{
					std::cout << "ERROR: Exact point 3d: 1 not found in mapExactPoints3d!!" << std::endl;
					break;
				}
				MapExactPoints3d::iterator foundPoint3d_2;
				if ( ( foundPoint3d_2 = mapExactPoints3d.find(face.vertex(2))) == mapExactPoints3d.end())
				{
					std::cout << "ERROR: Exact point 3d: 2 not found in mapExactPoints3d!!" << std::endl;
					break;
				}

				//Aggiungo il triangolo alla mesh
				meshOutput.push_back(Triangle(foundPoint3d_0->second, foundPoint3d_1->second, foundPoint3d_2->second));



#ifdef CUTTERMESHER_TEST_DEBUG
//TEST: cerco se i lati del triangolo sono in intersectSegments
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_0->second) && (end3d == foundPoint3d_1->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_1->second) && (end3d == foundPoint3d_0->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_0->second) && (end3d == foundPoint3d_2->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_2->second) && (end3d == foundPoint3d_0->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_1->second) && (end3d == foundPoint3d_2->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
for (std::list<Segment>::iterator segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;
	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	if ((start3d == foundPoint3d_2->second) && (end3d == foundPoint3d_1->second))
	{
		intersectSegments.erase(segmentIter);
		break;
	}
}
#endif



				//std::cout << "F\n";
			} //Chiudo if su isInArrangment
			//else
			//{
			//	std::cout << "N\n";
			//}

		} //Chiudo ciclo for sui triangoli della mesh


#ifdef CUTTERMESHER_TEST_DEBUG
vettorelisteseg.push_back(puntiFrontiera);
vettorelisteseg.push_back(puntiInterni);

//vettorelisteseg.push_back(triangoliMesh);

std::cout << "\nend Test Cerco frontiera: intersectSegments.size(): " << intersectSegments.size() << std::endl;
{
ListCSegment2d listasegtest;
ListSegment_2 listaSegs_2;

//Proietto i segmenti 3d in intersectSegments sul piano plane
std::list<Segment>::iterator segmentIter;
for (segmentIter = intersectSegments.begin(); segmentIter != intersectSegments.end(); ++segmentIter)
{
	Segment segment3d = *segmentIter;

	Point start3d = segment3d.start();
	Point end3d = segment3d.end();
	KPoint2 start = plane.to_2d(start3d);
	KPoint2 end = plane.to_2d(end3d);

	//Se il segmento non e' degenere lo aggiungo ad outlineSegements
	//if ((start.x()!=end.x()) || (start.y()!=end.y()))
	if ( start!=end )
	{
		Segment_2 segment2d(Point_2(start.x(), start.y()), Point_2(end.x(), end.y()));
		outlineSegements.push_back(segment2d);

CSegment2d seg2d;
seg2d.startX =  CGAL::to_double(start.x());
seg2d.startY =  CGAL::to_double(start.y());
seg2d.endX =  CGAL::to_double(end.x());
seg2d.endY =  CGAL::to_double(end.y());
listasegtest.push_back(seg2d);
listaSegs_2.push_back(segment2d);

std::cout << "XXXX: [" <<segment2d.start().x() << ", " << segment2d.start().y() << " ; ";
std::cout << segment2d.end().x() << ", " << segment2d.end().y() << "] " << std::endl;

	}

}
vettorelisteseg.push_back(listasegtest);
}
#endif

				//Metodo precedente che potrebbe inserire un errore a causa della creazione piu' volte dei stessi punti inesatti,
				//sostituito dal ciclo precendente
				/*
				//Ciclo le facce della mesh e creo i triangoli sul piano e li aggiungo in meshOutput
				for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit)
				{
					Face face = *fit;

					if (face.is_in_domain()) {

						//Estraggo i punti 2d (Point2 kernel non esatto)
						Point2 points[3];
						points[0] = face.vertex(0)->point();
						points[1] = face.vertex(1)->point();
						points[2] = face.vertex(2)->point();

						//Creo i KPoint2 2d (KPoint2 kernel esatto)
						KPoint2 p0_2d(points[0].x(), points[0].y());
						KPoint2 p1_2d(points[1].x(), points[1].y());
						KPoint2 p2_2d(points[2].x(), points[2].y());

						//Recupero i punti esatti corrispondenti alla frontiera della mesh, se ci sono
						MapExactPoints::iterator foundPair;
						//if ( (foundPair = mapExactPoints.find(points[0])) != mapExactPoints.end())
						if ( (foundPair = mapExactPoints.find(face.vertex(0))) != mapExactPoints.end())
						{
							//std::cout << "*";
							p0_2d = foundPair->second;
						}
						//if ( (foundPair = mapExactPoints.find(points[1])) != mapExactPoints.end())
						if ( (foundPair = mapExactPoints.find(face.vertex(1))) != mapExactPoints.end())
						{
							//std::cout << "*";
							p1_2d = foundPair->second;
						}
						//if ( (foundPair = mapExactPoints.find(points[2])) != mapExactPoints.end())
						if ( (foundPair = mapExactPoints.find(face.vertex(2))) != mapExactPoints.end())
						{
							//std::cout << "*";
							p2_2d = foundPair->second;
						}

						//proietto il punto 2d sul piano 3d (Point kernel esatto)
						Point p0_3d = plane.to_3d(p0_2d);
						Point p1_3d = plane.to_3d(p1_2d);
						Point p2_3d = plane.to_3d(p2_2d);

						//Aggiungo il triangolo alla mesh
						meshOutput.push_back(Triangle(p0_3d, p1_3d, p2_3d));

					}
				}
				*/

	}
	else
	{
		std::cout << "WARNING!! getFacesHolesHierarchy return 0 faces" << std::endl;
	}

	std::cout << "meshOutput total triangles: " << meshOutput.size() << std::endl;

	timer.stop();
	std::cout << "Construction took " << timer.time() << " seconds." << std::endl << std::endl;

	return meshOutput;

}


#ifdef CUTTERMESHER_TEST_DEBUG
ListCSegment2d findIncongruencies(ListSegment_2 contorno)
{
	std::cout << "START findIncongruencies" << std::endl;

	ListCSegment2d listasegtest;
	std::list<KPoint2> pointsNotFounded;

	ListSegment_2::iterator iter1;
	for(iter1=contorno.begin(); iter1!= contorno.end(); ++iter1)
	{
		Segment_2 a = *iter1;

		bool found = false;
		ListSegment_2::iterator iter2;
		//Search a.start
		for(iter2=contorno.begin(); iter2!= contorno.end(); ++iter2)
		{
			Segment_2 b = *iter2;
			if ( ((a.start() == b.start()) || (a.start() == b.end())) && (a != b) )
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			pointsNotFounded.push_back( a.start() );
		}

		found = false;
		//Search a.end
		for(iter2=contorno.begin(); iter2!= contorno.end(); ++iter2)
		{
			Segment_2 b = *iter2;
			if ( ((a.end() == b.start()) || (a.end() == b.end())) && (a != b) )
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			pointsNotFounded.push_back (a.end() );
		}

	}


	std::list<KPoint2>::iterator iterOut;
	for(iterOut=pointsNotFounded.begin(); iterOut!= pointsNotFounded.end(); ++iterOut)
	{
		KPoint2 p = *iterOut;
		CSegment2d s1;
		s1.startX =  CGAL::to_double(p.x()+0.01);
		s1.startY =  CGAL::to_double(p.y()+0.01);
		s1.endX =  CGAL::to_double(p.x()-0.01);
		s1.endY =  CGAL::to_double(p.y()-0.01);
		listasegtest.push_back(s1);

		CSegment2d s2;
		s2.startX =  CGAL::to_double(p.x()+0.01);
		s2.startY =  CGAL::to_double(p.y()-0.01);
		s2.endX =  CGAL::to_double(p.x()-0.01);
		s2.endY =  CGAL::to_double(p.y()+0.01);
		listasegtest.push_back(s2);
		std::cout << "* " << p.x() << " , " << p.y() << "\n";
	}

	if (pointsNotFounded.size() != 0)
	{
		std::list<KPoint2>::iterator iterOutx;
		iterOut=pointsNotFounded.begin();
		KPoint2 p1 = *iterOut;
		++iterOut;
		KPoint2 p2 = *iterOut;
		std::cout << "p1 == p2: "<<  (p1.x() ==p2.x() ) << " " << (p1.y() ==p2.y()) << "\n";
		std::cout << "p1 id: " << p1.id() << "\n";
		std::cout << "p2 id: " << p2.id() << "\n";
	}

	std::cout << "END findIncongruencies" << std::endl;

	return listasegtest;
}
#endif

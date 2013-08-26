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

#ifndef SEGMENTSARRANGEMENT_H_
#define SEGMENTSARRANGEMENT_H_

//log some messages
//#define SEGMENTSARRANGEMENT_LOG

//Esegue Step 3 Arrangment
#define SEGMENTSARRANGEMENT_SET_FACELEVEL_AND_ISFACE

//Esegue Step 4 Arrangment
//#define SEGMENTSARRANGEMENT_FIND_SEEDS

//Definisce polygons per check appartenenza facce e funzioni
//#define SEGMENTSARRANGEMENT_USE_POLYGON


#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
//#include <CGAL/Simple_cartesian.h>

//Per studio contorni
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_on_surface_with_history_2.h>
//#include <CGAL/Arrangement_with_history_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Timer.h>

//For Location query points
#include <CGAL/Arr_batched_point_location.h>

//2D Intersection of Curves
#include <CGAL/Sweep_line_2_algorithms.h>

//typedef CGAL::Cartesian<double>                           Kernel;
typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
typedef Kernel::Segment_2                                   Segment_2;
typedef CGAL::Arr_segment_traits_2<Kernel>                  Traits_2;
typedef Traits_2::Point_2                                   Point_2;
typedef CGAL::Arrangement_2<Traits_2>                       Arrangement_2;
typedef Kernel::Line_2                                      Line_2;

//For Location query points
typedef std::pair<Point_2, CGAL::Object>  QueryResult;
typedef std::map<Point_2, CGAL::Object>   MapQueryResult;

#ifdef SEGMENTSARRANGEMENT_USE_POLYGON
	#include <CGAL/Polygon_2.h>
	typedef Kernel::Point_2            Kernel_Point_2;
	typedef CGAL::Polygon_2<Kernel>    Polygon_2;
#endif

/*
//------------------------------------------------------------------
//2d Snap rounding
#include <CGAL/Snap_rounding_traits_2.h>
#include <CGAL/Snap_rounding_2.h>
typedef CGAL::Snap_rounding_traits_2<Kernel>     TraitsSnap;
typedef Kernel::Segment_2                        Segment_2;
typedef Kernel::Point_2                          KKPoint_2;
typedef std::list<Segment_2>                     Segment_list_2;
typedef std::list<KKPoint_2>                     Polyline_2;
typedef std::list<Polyline_2>                    Polyline_list_2;
//------------------------------------------------------------------
*/

/*
//CODICE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISET
struct classSegComp {
  bool operator() (const Segment_2& s1, const Segment_2& s2) const
  {
	 //TODO: confrontare i segmenti in modo piu' utile dato che potrebbero essere tutti uguali ma sicuramente in posizione diverse
	 // Usare metodi compare : http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Arrangement_on_surface_2_ref/Chapter_intro.html
	  return s1.squared_length() < s2.squared_length();
  }
};
*/

//Struttura per memorizzare la gerarchia di faces and holes
struct FaceHoleInfo {
		//CODICE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISET
		//std::multiset<Segment_2,classSegComp> segments; //Segmenti
	std::list<Segment_2> segments; //Segmenti ordinati
	bool isFace; //true se is a face ad not a hole
	unsigned int faceLevel; //0 if is a face of first level (it doens't have parentFace)
	std::vector<FaceHoleInfo*> holes; //vettore di puntatori FaceHoleInfo che rappresentano i buchi della faccia
	FaceHoleInfo *parentFace; //Vettore alla faccia che lo contiene
	Arrangement_2::Face faceArrangement; //Face arrangement della faccia
	Arrangement_2::Face_const_handle faceArrangmentHandle;  //Face handle arrangement della faccia
	Point_2 seed; //Punto Seed se la faccia e' un hole
	#ifdef SEGMENTSARRANGEMENT_USE_POLYGON
		Polygon_2 polygon; //Poligono che rappresenta la faccia costruito con i segments
	#endif
};


class SegmentsArrangement {

	Arrangement_2 arr;
	std::vector<FaceHoleInfo> allFacesHolesVector;
	MapQueryResult mapLocateQueryResult;

  public:

	SegmentsArrangement();
	~SegmentsArrangement();

	static bool compareFacesList(std::list<Segment_2> face1, std::list<Segment_2> face2);
	static std::list<Segment_2> getFaceInHalfedgeConstCirculatorFaceList(Arrangement_2::Ccb_halfedge_const_circulator &circ, bool counterClockwise);

	//CODICE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISET
	//bool compareFacesSets(std::multiset<Segment_2,classSegComp> face1, std::multiset<Segment_2,classSegComp> face2);
	//std::multiset<Segment_2,classSegComp> getFaceInHalfedgeConstCirculatorFace(Arrangement_2::Ccb_halfedge_const_circulator &circ);
	//std::multiset<Segment_2,classSegComp> getFaceInHalfedgeConstCirculatorHole(Arrangement_2::Ccb_halfedge_const_circulator &circ);

	#ifdef SEGMENTSARRANGEMENT_SET_FACELEVEL_AND_ISFACE
		static void recursiveVisitFacesHolesHierarchy(FaceHoleInfo *faceInfo, short int faceLevel);
	#endif

	bool isInArrangment(Point_2 point);
	#ifdef SEGMENTSARRANGEMENT_USE_POLYGON
		unsigned short int isInArrangmentRecursive(FaceHoleInfo *faceInfo, Point_2 &point);
	#endif

	void locatePoints(std::list<Point_2> points);

	std::vector<FaceHoleInfo> getFacesHolesHierarchy(std::vector<Segment_2> outlineSegements);

	std::vector<FaceHoleInfo> getAllFacesHolesVectorInObject();

	void arrangmentSimplification(std::vector<FaceHoleInfo> &facesHolesVector);

};


#endif /* SEGMENTSARRANGEMENT_H_ */

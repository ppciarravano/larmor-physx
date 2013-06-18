/*
 * Project name: Larmor-Physx
 * Released: 14 June 2013
 * Author: Pier Paolo Ciarravano
 * http://www.larmor.com
 *
 * License: This project is released under the Qt Public License (QPL - OSI-Approved Open Source license).
 * http://opensource.org/licenses/QPL-1.0
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 */

#ifndef CUTTERMESHER_H_
#define CUTTERMESHER_H_

//Use Delaunay mesh triangulation for cutted faces: it require more cpu time but it give better results
#define CUTTERMESHER_USE_DELAUNAY

//Use only exact precision for face building
//#define CUTTERMESHER_USE_ONLY_EXACT_FACE_BUILDER

//log some messages
//#define CUTTERMESHER_LOG
#define CUTTERMESHER_LOG_TIME

//Esegue TEST E DEBUG
//#define CUTTERMESHER_TEST_DEBUG

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <CGAL/Timer.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

//#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/squared_distance_3.h>
//#include <CGAL/Bbox_3.h>

//For Mesh generation
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>

#include <CGAL/Constrained_triangulation_plus_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef Kernel::Plane_3 Plane;
typedef Kernel::Vector_3 Vector;
typedef Kernel::Segment_3 Segment;
typedef Kernel::FT FT;
typedef Kernel::Ray_3 Ray;
typedef Kernel::Line_3 Line;
typedef Kernel::Point_3 Point;
typedef Kernel::Triangle_3 Triangle;
typedef Kernel::Direction_3 Direction;
typedef CGAL::Bbox_3 Bbox;

typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<Kernel,Iterator> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

typedef Tree::Object_and_primitive_id Object_and_primitive_id;
typedef Tree::Primitive_id Primitive_id;

//#define CGAL_USE_CORE

	//#include <CGAL/Filtered_kernel.h>
	//typedef CGAL::Simple_cartesian<double> KKSC;
	//typedef CGAL::Filtered_kernel_adaptor<KKSC> KK;
//#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
//typedef CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt  KK;
//typedef CGAL::Exact_predicates_exact_constructions_kernel   KK;
//#include <CGAL/Real_embeddable_traits.h>
/* //da usare solo per cutMesh_UsingDelaunayMesh, usando la generazione della mesh Delaunay
//Se uso Simple_cartesian la generazione della mesh e' piu' veloce e non dovrei avere problemi di precisione
typedef CGAL::Simple_cartesian<double> KK;
//typedef CGAL::Exact_predicates_inexact_constructions_kernel KK;
typedef CGAL::Triangulation_vertex_base_2<KK> Vb;
typedef CGAL::Delaunay_mesh_face_base_2<KK> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<KK, Tds> CDT;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDT> Criteria;
typedef CGAL::Delaunay_mesher_2<CDT, Criteria> Mesher;
typedef CDT::Vertex_handle Vertex_handle;
//typedef CDT::Geom_traits::Point_2 Point2;
typedef CDT::Point Point2;
typedef CDT::Face Face;
*/

//Used for faceBuilder_inexact
//typedef CGAL::Simple_cartesian<double> KK; //potrebbe dare un errore CGAL ERROR: precondition violation
typedef CGAL::Exact_predicates_inexact_constructions_kernel KK;
//typedef CGAL::Exact_predicates_exact_constructions_kernel KK;
typedef CGAL::Triangulation_vertex_base_2<KK> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<KK>    Fb;
//typedef CGAL::Delaunay_mesh_face_base_2<KK> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Constrained_triangulation_2<KK, Tds> CDT;
typedef CDT::Vertex_handle Vertex_handle;
typedef CDT::Point Point2;
typedef CDT::Face Face;

//Used for faceBuilder_exact
typedef CGAL::Exact_predicates_exact_constructions_kernel KKEX;
typedef CGAL::Triangulation_vertex_base_2<KKEX> VbEX;
typedef CGAL::Constrained_triangulation_face_base_2<KKEX>    FbEX;
typedef CGAL::Triangulation_data_structure_2<VbEX, FbEX> TdsEX;
typedef CGAL::Constrained_triangulation_2<KKEX, TdsEX> CDTEX;
typedef CDTEX::Vertex_handle Vertex_handleEX;
typedef CDTEX::Point Point2EX;
typedef CDTEX::Face FaceEX;

//Used for faceBuilder_Delaunay
typedef CGAL::Exact_predicates_inexact_constructions_kernel KKDY;
typedef CGAL::Triangulation_vertex_base_2<KKDY> VbDY;
typedef CGAL::Delaunay_mesh_face_base_2<KKDY> FbDY;
typedef CGAL::Triangulation_data_structure_2<VbDY, FbDY> TdsDY;
//typedef CGAL::No_intersection_tag  Itag1; //Default
//typedef CGAL::Exact_predicates_tag Itag2;
//typedef CGAL::Exact_intersections_tag Itag3;
typedef CGAL::Constrained_Delaunay_triangulation_2<KKDY, TdsDY> CDTDY_DEFAULT;
typedef CGAL::Constrained_triangulation_plus_2<CDTDY_DEFAULT> CDTDY;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDTDY> Criteria;
//typedef CGAL::Delaunay_mesher_2<CDTDY, Criteria> Mesher;
typedef CDTDY::Vertex_handle Vertex_handleDY;
//typedef CDT::Geom_traits::Point_2 Point2DY;
typedef CDTDY::Point Point2DY;
typedef CDTDY::Face FaceDY;
typedef CDTDY::Edge EdgeDY;
typedef CDTDY::Context ContextDY; //from Constrained_triangulation_plus_2
typedef CDTDY::Vertices_in_constraint_iterator Vertices_in_constraint_iteratorDY;  //from Constrained_triangulation_plus_2

//typedef KK::Triangle_2 Triangle2D;

//#include <CGAL/Cartesian_converter.h>
//typedef CGAL::Cartesian_converter<Kernel,KK>    Kernel_to_KK;

typedef Kernel::Point_2 KPoint2;

#include "SegmentsArrangement.h"

//Mappa che serve per ritrovare i punti esatti della frontiera esatti
//typedef std::map<Point2, KPoint2>   MapExactPoints;
//typedef std::pair<Point2, KPoint2>  PairPointsInexactExact;
typedef std::map<Vertex_handle, KPoint2>   MapExactPoints;
typedef std::pair<Vertex_handle, KPoint2>  PairPointsInexactExact;
//for faceBuilder_Delaunay
typedef std::map<Vertex_handleDY, KPoint2>   MapExactPointsDY;
typedef std::pair<Vertex_handleDY, KPoint2>  PairPointsInexactExactDY;

//Mappa che mappa Point_2 in Vertex_handle, per non creare piu' volte i stessi punti inesatti della mesh CDT,
//serve quando dai segmenti dell'arrangment si inseriscono i constraint per la mesh
typedef std::map<Point_2,Vertex_handle>   MapInexactPoints;
typedef std::pair<Point_2,Vertex_handle>  PairPointsExactInexact;
//for faceBuilder_Delaunay
typedef std::map<Point_2,Vertex_handleDY>   MapInexactPointsDY;
typedef std::pair<Point_2,Vertex_handleDY>  PairPointsExactInexactDY;

//Mappa che mappa Vertex_handle della mesh CDT 2d nel punto Point in 3d
//Usato solo in cutMesh_UsingDelaunayMesh
typedef std::map<Vertex_handle, Point>   MapExactPoints3d;
typedef std::pair<Vertex_handle, Point>  PairPointsInexactExact3d;


//-----------------------------------------------------------------------
//Strutture per memorizzare gli attributi del taglio di un triangolo
#define CUTTERMESHER_TRIANGLE_NO_CUTTED      1
#define CUTTERMESHER_TRIANGLE_CUTTED         2
#define CUTTERMESHER_TRIANGLE_IN_CUT_PLANE   3
struct TriangleInfo {

	//int id;

	//CUTTERMESHER_TRIANGLE_NO_CUTTED    : 1: triangolo mesh originale non tagliato
	//CUTTERMESHER_TRIANGLE_CUTTED       : 2: triangolo mesh che e' stato attraversato dal piano di taglio
	//CUTTERMESHER_TRIANGLE_IN_CUT_PLANE : 3: triangolo mesh superficie di taglio
	short int cutType;

};
typedef std::list<Triangle>      TrianglesList;     //Non utilizzato in CutterMesher.cpp
typedef std::list<TriangleInfo>  TrianglesInfoList; //Non utilizzato in CutterMesher.cpp
typedef std::pair<TrianglesList, TrianglesInfoList> MeshData;

typedef std::map<int, TriangleInfo>   MapIdTriangleInfo;
typedef std::pair<int, TriangleInfo>  PairIdTriangleInfo;

inline TriangleInfo getNewTriangleInfo(short int cutType);
void addInTrianglesInfoOutput(std::list<TriangleInfo> &trianglesInfoOutput, MapIdTriangleInfo &mapIdTriangleInfo, Triangle &t, short int cutType);
//-----------------------------------------------------------------------


#include "Custom.h"

#ifdef CUTTERMESHER_TEST_DEBUG
	typedef std::list<Segment_2> ListSegment_2;
	typedef std::vector<ListSegment_2> VectListSegment_2;
	ListCSegment2d findIncongruencies(ListSegment_2 contorno);
#endif

//TODO: metodo per estendere Triangle
//class TypedTriangle: public Triangle {
//  public:
//	unsigned int type;
//};


//bool isSignedDistancePositive(Plane plane, Point point);
inline bool isSignedDistancePositive(Plane &plane, Point point);

std::list<TriangleInfo> createNewTriangleInfoList(std::list<Triangle> &meshInput);

MeshData cutMesh(std::list<Triangle> &meshInput,  Plane plane, std::list<TriangleInfo> &trianglesInfoInput);

bool faceBuilder_inexact(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo);
bool faceBuilder_exact(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo);
bool faceBuilder_Delaunay(SegmentsArrangement &segmentsArrangment, TrianglesList &meshOutput, TrianglesInfoList &trianglesInfoOutput, Plane &plane, MapIdTriangleInfo &mapIdTriangleInfo);

//Deprecated: it doesn't work
std::list<Triangle> cutMesh_UsingDelaunayMesh(std::list<Triangle> &meshInput, Plane plane);

#endif /* CUTTERMESHER_H_ */

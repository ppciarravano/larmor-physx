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

#include "DelaunayVoronoi.h"

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <boost/unordered_map.hpp>
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
#include <CGAL/Cartesian.h>
#include <CGAL/squared_distance_3.h>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
//typedef CGAL::Simple_cartesian<double>   Kernel;
typedef Kernel::Triangle_3 Triangle;
typedef Kernel::Point_3 Point;
typedef Kernel::Plane_3 Plane;
typedef Kernel::Point_2 KPoint2;

struct TriangleVisitedInfoMC {

	Triangle *triangle;
	bool intersected;

};

//For Box Collision
typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TriangleVisitedInfoMC*> BoxInt;

#define DISJOINTMESH_TRIANGLE_SCALE_VALUE 0.999999999
//Scale triangle, used for improve do_intersect results: scale the triangle on its plane, centred in its barycenter
inline Triangle triangleScale(Triangle t)
{
	Plane p = t.supporting_plane();
	KPoint2 a = p.to_2d(t.vertex(0));
	KPoint2 b = p.to_2d(t.vertex(1));
	KPoint2 c = p.to_2d(t.vertex(2));
	KPoint2 baryc((a.x() + b.x() +c.x())/3.0, (a.y() + b.y() +c.y())/3.0);
	//TODO: usare margine invece di scale
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


void reportSelfIntersectionCallback(const BoxInt& b1, const BoxInt& b2) {

	TriangleVisitedInfoMC *m1 = b1.handle();
	TriangleVisitedInfoMC *m2 = b2.handle();

	if ( (! m1->triangle->is_degenerate()) && (! m2->triangle->is_degenerate()) )
	{

		//if (CGAL::do_intersect( *(m1->triangle), *(m2->triangle) ) )
		if (CGAL::do_intersect( triangleScale(*(m1->triangle)), triangleScale(*(m2->triangle)) ) )
		{
			//std::cout << " INTERSECT!! " << std::endl;
			m1->intersected = true;
			m2->intersected = true;
		}

		/*
		CGAL::Object intersectionResult = CGAL::intersection(*(m1->triangle), *(m2->triangle));
		if (const CGAL::Triangle_3<Kernel> *itriangle = CGAL::object_cast<CGAL::Triangle_3<Kernel> >(&intersectionResult))
		{
			std::cout << "INTERSECT section is a Triangle!!" << std::endl;
			m1->intersected = true;
			m2->intersected = true;
		}

		//if (const CGAL::Point_3<Kernel> *ipoint = CGAL::object_cast<CGAL::Point_3<Kernel> >(&intersectionResult))
		//{
		//	std::cout << "Point_3" << std::endl;
		//}

		//if (const CGAL::Segment_3<Kernel> *isegment = CGAL::object_cast<CGAL::Segment_3<Kernel> >(&intersectionResult))
		//{
		//	std::cout << "Segment_3" << std::endl;
		//}

		//if (const std::vector < CGAL::Point_3<Kernel> > *ivectpoint = CGAL::object_cast<std::vector < CGAL::Point_3<Kernel> > >(&intersectionResult))
		//{
		//	std::cout << "Vect Point_3" << std::endl;
		//}
		*/
	}

}


TrianglesList meshRemoveIntersectedTriangles(TrianglesList &triangles)
{
	CGAL::Timer timer;
	timer.start();

	TrianglesList result;

	//Collision boxes
	std::vector<BoxInt> boxes;

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = triangles.begin(); triangleIter != triangles.end(); ++triangleIter)
	{
		//Triangle t = *triangleIter;

		TriangleVisitedInfoMC *tvi = new TriangleVisitedInfoMC;
		tvi->triangle = &(*triangleIter); //Use the pointer, it should not change into the container, since there aren't new added elements
		tvi->intersected = false;

		boxes.push_back( BoxInt( (*triangleIter).bbox(), tvi ));
	}


	//Do intersection
	CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), reportSelfIntersectionCallback);


	//cycle on boxes and build result and delete tvi
	std::vector<BoxInt>::iterator vectorBoxesIter;
	for(vectorBoxesIter = boxes.begin(); vectorBoxesIter != boxes.end(); ++vectorBoxesIter)
	{
		BoxInt boxInt = *vectorBoxesIter;
		TriangleVisitedInfoMC *tviHandled = boxInt.handle();
		if ((!(tviHandled->intersected)) && (!(tviHandled->triangle->is_degenerate())))
		{
			Triangle t = *(tviHandled->triangle);
			result.push_back(t);
		}

		delete tviHandled;
	}


	timer.stop();
	std::cout << "Total meshRemoveIntersectedTriangles time: " << timer.time() << std::endl;

	return result;
}


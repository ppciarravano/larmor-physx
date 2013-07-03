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

#ifndef CUSTOM_H_
#define CUSTOM_H_

#include <list>
#include <vector>
#include <iostream>
//using namespace std;

struct CPoint3d {
	double x;
	double y;
	double z;
};

struct CSegment2d {
	double startX;
	double startY;
	double endX;
	double endY;
};

struct CSegment3d {
	double startX;
	double startY;
	double startZ;
	double endX;
	double endY;
	double endZ;
};

struct CTriangle3d {
	double aX;
	double aY;
	double aZ;
	double bX;
	double bY;
	double bZ;
	double cX;
	double cY;
	double cZ;
	//For cut type
	short int type;
};

typedef std::vector<CPoint3d> VectCPoint3d;

typedef std::list<CSegment2d> ListCSegment2d;
typedef std::vector<ListCSegment2d> VectListCSegment2d;

typedef std::list<CSegment3d> ListCSegment3d;
typedef std::vector<ListCSegment3d> VectListCSegment3d;

typedef std::list<CTriangle3d> ListCTriangle3d;
typedef std::vector<ListCTriangle3d> VectListCTriangle3d;


#endif /* CUSTOM_H_ */

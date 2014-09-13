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

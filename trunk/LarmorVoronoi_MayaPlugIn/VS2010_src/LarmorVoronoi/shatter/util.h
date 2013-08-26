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

#ifndef UTIL_H_
#define UTIL_H_

#include <list>
#include <vector>
#include <iostream>
//using namespace std;

#include "CutterMesher.h"
#include "Custom.h"

//Funzioni per convertire
std::list<Triangle> toTriangles(ListCTriangle3d triangles);
ListCTriangle3d fromTriangles(std::list<Triangle> triangles);
ListCTriangle3d fromTriangles(std::list<Triangle> triangles, std::list<TriangleInfo> trianglesInfoInput);
void fromTriangles(ListCTriangle3d &output, std::list<Triangle> triangles, std::list<TriangleInfo> trianglesInfoInput);

//Funzioni per leggere la mesh
std::list<Triangle> readPlyMeshToTriangles(const char* meshFile);
std::list<Triangle> readOffMeshToTriangles(const char* meshFile);
std::list<Triangle> readObjMeshToTriangles(const char* meshFile);


bool equalCSegment2d(CSegment2d a, CSegment2d b);

void printDouble(double val);

//DEPRECATED: USARE readMeshToTriangles
ListCTriangle3d readMesh(const char* meshFile);

#endif /* UTIL_H_ */

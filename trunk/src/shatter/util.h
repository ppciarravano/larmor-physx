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

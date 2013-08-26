/*
 * Project name: LarmorVoronoi (Larmor-Physx)
 * Mesh Voronoi shatter Maya Plug-in
 * Version 0.2 (for Maya 2012) Build 72
 * Released: 28 July 2013
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

#ifndef LARMORVORONOI_H_
#define LARMORVORONOI_H_

#include <maya/MSimple.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MIOStream.h>
#include <maya/MPointArray.h>
#include <maya/MFnTransform.h>
#include <maya/MColorArray.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <math.h>

#include "shatter/CutterMesher.h"
#include "shatter/Custom.h"
#include "shatter/util.h"
#include "shatter/DelaunayVoronoi.h"

#include "util/check_version.h"

//Plugin version
#define PLUGIN_INFO "Vers.1.0Beta Build 72 (25/08/13) - Author: Pier Paolo Ciarravano www.larmor.com"
#define PLUGIN_VERSION "1.0Beta-72"

//License and version check
#define PRODUCT_NAME "LarmorVoronoiMayaPlugin_win2012x64"
#define PRODUCT_VERSION_BUILD "1.0Beta-72"

typedef std::map<Point, MPoint>   MapPointMPoint;
typedef std::pair<Point, MPoint>  PairPointMPoint;

class LarmorVoronoi: public MPxCommand
{
	public:
		LarmorVoronoi();
		virtual	~LarmorVoronoi(); 
		MStatus doIt( const MArgList& args );
		static void* creator();
};

void getPointArrayFromMELvectorArray(MString arrayVectorName, MPointArray &pointsArray);

int buildMeshTrianglesFromSelection(std::list<Triangle> &meshTrianglesToShatter);

void buildVerticesAndConnections(MeshData &meshData, 
									int &numTriagles, 
									MPointArray &pieceVertices, 
									MIntArray &pieceConnections,
									MIntArray &triangleCounts,
									MIntArray &faceTypes);

#endif /* LARMORVORONOI_H_ */


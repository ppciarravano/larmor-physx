/*****************************************************************************
 * Larmor-Physx Version 1.0 2013
 * Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of LarmorVoronoi Maya Plugin (http://code.google.com/p/larmor-physx/).
 *
 * LarmorVoronoi Maya Plugin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LarmorVoronoi Maya Plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LarmorVoronoi Maya Plugin. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 * $Id$
 *
 ****************************************************************************/

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
#define PLUGIN_INFO "Vers.1.0.1Beta Build 73 (02/10/13) - Author: Pier Paolo Ciarravano www.larmor.com"
#define PLUGIN_VERSION "1.0.1Beta-73"

//License and version check
#define PRODUCT_NAME "LarmorVoronoiMayaPlugin_win2014x64"
#define PRODUCT_VERSION_BUILD "1.0.1Beta-73"

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


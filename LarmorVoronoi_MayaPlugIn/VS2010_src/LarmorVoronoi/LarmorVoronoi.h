/*
 * Project name: LarmorVoronoi (Larmor-Physx)
 * Mesh Voronoi shatter Maya Plug-in
 * Version 0.1 (for Maya 2012 64 bits)
 * Released: 2 July 2013
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

void buildVerticesAndConnections(MeshData &meshData, 
									int &numTriagles, 
									MPointArray &pieceVertices, 
									MIntArray &pieceConnections,
									MIntArray &triangleCounts,
									MIntArray &faceTypes);



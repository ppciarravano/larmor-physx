/*
 * Project name: LarmorVoronoi (Larmor-Physx)
 * Mesh Voronoi shatter Maya Plug-in
 * Version 1.0Beta (for Maya 2012)
 * Released: 18/08/2013
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

#include "LarmorVoronoi.h"

// Use helper macro to register a command with Maya.
//DeclareSimpleCommand( LarmorVoronoi, PLUGIN_INFO, "2012");

LarmorVoronoi::LarmorVoronoi()
{
	//std::cout << "Init LarmorVoronoi" << std::endl;
}

LarmorVoronoi::~LarmorVoronoi()
{
	//std::cout << "Destroy LarmorVoronoi" << std::endl;
}

void* LarmorVoronoi::creator()
{
	//std::cout << "Create LarmorVoronoi" << std::endl;
	return new LarmorVoronoi();
}

MStatus initializePlugin( MObject obj )
{
	//std::cout << "initializePlugin LarmorVoronoi" << std::endl;

	//Send Http async version check
	// Get Maya version e API version
	MString mayaVersion = MGlobal::mayaVersion();
	std::string mayaVersionString = mayaVersion.asUTF8();
	int apiVersion = MGlobal::apiVersion();
	std::stringstream productInfoSS;
	productInfoSS << "mayaVersion:" << mayaVersionString << " - apiVersion:" << apiVersion;
	std::string productInfo = productInfoSS.str();
	//std::cout << "send version check" << std::endl;
	LarmorCheckProductVersion(PRODUCT_NAME, PRODUCT_VERSION_BUILD, productInfo);
	
	// init plugin
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_INFO, PLUGIN_VERSION, "Any");
	status = plugin.registerCommand( "LarmorVoronoi", LarmorVoronoi::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	//std::cout << "uninitializePlugin LarmorVoronoi" << std::endl;

	MStatus   status;
	MFnPlugin plugin( obj );
	status = plugin.deregisterCommand( "LarmorVoronoi" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}


//
//	Description:
//		implements the MEL LarmorVoronoi command.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//      LarmorVoronoi [-np int_num_pieces | -p string_vector_array_name] [-d bool_disjoint] [-ex float_explode]
//      or LarmorVoronoi -vi float_facet_distance_multiple
//      int_num_pieces is the number of the voronoi cells (default 10).
//      If  -d true  the shatter separates the disjointed surfaces (default false).
//		string_vector_array_name is the global array vector nome from MEL
//      float_explode is the pieces distance if -ex is used
//      float_facet_distance_multiple is the multiple for the precision of volume and inertia calculation (e.g. 10, 100, 1000)
//
MStatus LarmorVoronoi::doIt( const MArgList& args )
{
	MGlobal::displayInfo("Start LarmorVoronoi...");
	if ((LarmorCheckProductVersion_getCode() != 1) && (LarmorCheckProductVersion_getCode() != -1))
	{
		MGlobal::displayInfo(LarmorCheckProductVersion_getMessage().c_str());
	}
	MGlobal::displayInfo("Use: LarmorVoronoi [-np int_num_pieces | -p string_vector_array_name] [-d bool_disjoint] [-ex float_explode]");
	MGlobal::displayInfo(" or: LarmorVoronoi -vi float_facet_distance_multiple");

	//MStatus stat = MS::kSuccess;
	MStatus stat;

	//Read the command arguments and initialize
	int numberPieces = 10;
	bool disjointMeshes = false;
	unsigned int index;
	bool doExplode = false;
	double explodeDistance = 1.0;
	bool doVolumeInertia = false;
	double facetDistanceMultiple = 0.0;
	//parameter for number of pieces
	index = args.flagIndex( "np", "npieces" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, numberPieces );
	}
	//parameter for disjoint meshes
	index = args.flagIndex( "d", "disjoint" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, disjointMeshes );
	}
	//parameter for name of array of vectors in MEL
	std::list<KDPoint> customPoints;
	index = args.flagIndex( "p", "points" );
	if( index != MArgList::kInvalidArgIndex )
	{
		MString arrayVectorName;
		stat = args.get( index+1, arrayVectorName );
		if ( MS::kSuccess == stat )
		{
			MString argsPointsMessage1("Reading array of points from MEL vector array variable: ");
			argsPointsMessage1 += arrayVectorName;
			MGlobal::displayInfo(argsPointsMessage1);

			MPointArray pointsArray;
			//Read the point using MEL function
			getPointArrayFromMELvectorArray(arrayVectorName, pointsArray);

			MString argsPointsMessage2("Read total number of point: ");
			argsPointsMessage2 += (int)pointsArray.length();
			MGlobal::displayInfo(argsPointsMessage2);

			//Build std::list<KDPoint> customPoints
			for (int idx = 0; idx < pointsArray.length(); idx++)
			{
				MPoint mp = pointsArray[idx];
				customPoints.push_back(KDPoint(mp.x, mp.y, mp.z));
				//std::cout << "KDPoint:" << mp.x << ", " << mp.y << ", "<< mp.z << std::endl;
			}
		}
	}
	//parameter for doExplode and explodeDistance
	index = args.flagIndex( "ex", "explode" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, explodeDistance );
		doExplode = true;

		MString argsExplodeMessage("Explode with distance: ");
		argsExplodeMessage += (double)explodeDistance;
		MGlobal::displayInfo(argsExplodeMessage);
	}
	//parameter for doVolumeInertia and facetDistance
	index = args.flagIndex( "vi", "volumeinertia" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, facetDistanceMultiple );
		doVolumeInertia = true;
		
		if (facetDistanceMultiple <= 0.0)
		{
			facetDistanceMultiple = 1.0;
			MGlobal::displayInfo("Facet distance multiple can't be <= 0.0, it will be 1.0");
		}

		MString argsVolumeMessage("Calculate Volume and Inertia with facet distance multiple: ");
		argsVolumeMessage += (double)facetDistanceMultiple;
		MGlobal::displayInfo(argsVolumeMessage);
	}
	// end read the command arguments and initialize

	
	//Store the mesh triangles for Larmor Voronoi shatter
	std::list<Triangle> meshTrianglesToShatter;

	//Build mesh triangles from Maya selection 
	int meshesSelected = buildMeshTrianglesFromSelection(meshTrianglesToShatter);
	
	//Main Shatter call to DelaunayVoronoi.cpp voronoiShatter
	if (meshesSelected != 0)
	{
		if (meshTrianglesToShatter.size() != 0)
		{
			MString message1("Total Meshes selected: ");
			message1 += (int)meshesSelected;
			MGlobal::displayInfo(message1);

			MString message2("Total triangles: ");
			message2 += (int)meshTrianglesToShatter.size();
			MGlobal::displayInfo(message2);


			//Calculate Volume and Inertia tensor
			if (doVolumeInertia)
			{
				//Calculate minimum bounding box dimension
				double minBBDimension = minimumBoundingDimension(meshTrianglesToShatter);
				double facetDistance = minBBDimension / facetDistanceMultiple;

				MString volumeMessage1("Calculating Volume and Inertia with minBBDimension: ");
				volumeMessage1 += (double)minBBDimension;
				MGlobal::displayInfo(volumeMessage1);
				MString volumeMessage2("and facet distance: ");
				volumeMessage2 += (double)facetDistance;
				MGlobal::displayInfo(volumeMessage2);

				//Computation
				std::vector<double> volumeInertia = internalVolumeAndInertiaMesh(meshTrianglesToShatter, facetDistance);
				double volume = volumeInertia.at(0);
				double inertia_a = volumeInertia.at(1);
				double inertia_b = volumeInertia.at(2);
				double inertia_c = volumeInertia.at(3);
				double inertia_aa = volumeInertia.at(4);
				double inertia_bb = volumeInertia.at(5);
				double inertia_cc = volumeInertia.at(6);
				
				//Return the volume and inertia tensor
				//Inertia tensor is:
				// ( (inertia_a, -inertia_bb, -inertia_cc),
				//   (-inertia_bb, inertia_b, -inertia_aa),
				//   (-inertia_cc, -inertia_aa, inertia_c) )
				// read result from MEL using:
				// float $vals[]; $vals = `LarmorVoronoi -vi 100`;
				MDoubleArray resultVolumeInertia;
				resultVolumeInertia.append( volume );
				resultVolumeInertia.append( inertia_a );
				resultVolumeInertia.append( inertia_b );
				resultVolumeInertia.append( inertia_c );
				resultVolumeInertia.append( inertia_aa );
				resultVolumeInertia.append( inertia_bb );
				resultVolumeInertia.append( inertia_cc );
				clearResult();
				setResult(resultVolumeInertia);
				//or use appendToResult(volume);				
				
				MString volumeMessage3("Mesh Volume is: ");
				volumeMessage3 += (double)volume;
				MGlobal::displayInfo(volumeMessage3);
				MString inertiaMessage("Mesh Inertia tensor is: ((");
				inertiaMessage += (double)inertia_a;
				inertiaMessage += ", -";
				inertiaMessage += (double)inertia_bb;
				inertiaMessage += ", -";
				inertiaMessage += (double)inertia_cc;
				inertiaMessage += "), (-";
				inertiaMessage += (double)inertia_bb;
				inertiaMessage += ", ";
				inertiaMessage += (double)inertia_b;
				inertiaMessage += ", -";
				inertiaMessage += (double)inertia_aa;
				inertiaMessage += "), (-";
				inertiaMessage += (double)inertia_cc;
				inertiaMessage += ", -";
				inertiaMessage += (double)inertia_aa;
				inertiaMessage += ", ";
				inertiaMessage += (double)inertia_c;
				inertiaMessage += "))";
				MGlobal::displayInfo(inertiaMessage);
				
				return stat;
			}


			//Check for minimum number of pieces to 2
			if ( (customPoints.size() < 2) && (numberPieces < 2) )
			{
				setResult( "Minimum number of pieces is 2!\n" );
				return stat;
			}

			//Run Voronoi shatter
			MString argsMessage("Execute Voronoi Shatter with number pieces: ");
			argsMessage += (int)numberPieces;
			argsMessage += " and disjoint meshes:";
			argsMessage += (bool)disjointMeshes;
			MGlobal::displayInfo(argsMessage);

			std::list<MeshData> shatterMeshesNoCentered;
			//Do Voronoi shatter
			if (customPoints.size() >= 2) 
			{
				//Use customPoints
				MGlobal::displayInfo("Running Voronoi shatter using custom points ...");
				TrianglesInfoList cutInfo = createNewTriangleInfoList(meshTrianglesToShatter);
				MeshData meshData(meshTrianglesToShatter, cutInfo);
				shatterMeshesNoCentered = voronoiShatter(meshData, customPoints);
				if (disjointMeshes)
				{
					//Disjoint meshes
					shatterMeshesNoCentered = disjointNonContiguousListMeshes(shatterMeshesNoCentered);
				}
			}
			else
			{
				//Use uniformDistributionPoints in bounding box
				MGlobal::displayInfo("Running Voronoi shatter using uniform distribution points on bounding box ...");
				shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(meshTrianglesToShatter, numberPieces, disjointMeshes);
			}			
			
			//Center meshes in their barycenters
			std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);
			MGlobal::displayInfo("Voronoi statter complete");


			//Barycenter for exploding
			MVector meshBarycenter;
			if (doExplode)
			{
				TrianglesInfoList cutInfo = createNewTriangleInfoList(meshTrianglesToShatter);
				MeshData meshData(meshTrianglesToShatter, cutInfo);
				Point mb = getMeshBarycenter(meshData);
				meshBarycenter = MVector( CGAL::to_double(mb.x()), CGAL::to_double(mb.y()), CGAL::to_double(mb.z()));
				std::cout << "meshBarycenter for explode:" << meshBarycenter.x << ", " << meshBarycenter.y << ", "<< meshBarycenter.z << std::endl;
			}

			//Creating Maya meshes
			//Create Group
			MFnDagNode dagFn;
			MObject transform;
			transform = dagFn.create( "transform", "ShatterMeshesGroup" );
			dagFn.setObject( transform );
			
			int numPieces = 0;
			std::list<TranslatedMeshData>::iterator meshDataInter;
			for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
			{
				TranslatedMeshData meshDataPartTranslated = *meshDataInter;
				numPieces++;

				//Build vertices and connections
				int numTriangles;
				MPointArray pieceVertices;
				MIntArray pieceConnections;
				MIntArray trianglePolyCounts;
				MIntArray faceTypes;
				buildVerticesAndConnections(meshDataPartTranslated.first,
												numTriangles,
												pieceVertices,
												pieceConnections,
												trianglePolyCounts,
												faceTypes);
				
				MString message3("New mesh N:");
				message3 += (int)numPieces;
				message3 += " - num triangles:"; 
				message3 += (int)numTriangles;
				MGlobal::displayInfo(message3);

				//Create new Maya Mesh
				MFnMesh newMeshFn;
				MObject objTransform;
				objTransform = newMeshFn.create( pieceVertices.length(),
													numTriangles,
													pieceVertices,
													trianglePolyCounts,
													pieceConnections, 
													MObject::kNullObj, 
													&stat );
				
				if(!stat)
				{
					stat.perror( "Unable to create mesh" );
					continue;
				}
				
				//Set Face colors
				//TODO: use setColors and assignColors: it should be fast
				MColor red(1.0f, 0.0f, 0.0f);
				MColor blue(0.0f, 0.0f, 1.0f);
				for (int j = 0; j < numTriangles; j++)
				{
					if (faceTypes[j] == 0)
					{
						newMeshFn.setFaceColor(blue, j);
					}
					else
					{
						newMeshFn.setFaceColor(red, j);
					}
				}

				//TODO: set mesh normals
				// using MFnMesh::setVertexNormals and MFnMesh::setFaceVertexNormals
				
				//CGAL Point of translation for the barycenters
				Point originObj = meshDataPartTranslated.second;
				double cm_x = CGAL::to_double(originObj.x());
				double cm_y = CGAL::to_double(originObj.y());
				double cm_z = CGAL::to_double(originObj.z());
				MVector mayaVector(cm_x, cm_y, cm_z);
				//Translate the mesh in the proper position
				MFnTransform objMFnTransform(objTransform);
				if (doExplode)
				{
					//exploding center in meshBarycenter
					objMFnTransform.setTranslation(mayaVector + (mayaVector - meshBarycenter)*explodeDistance, MSpace::kTransform);
				}
				else
				{
					objMFnTransform.setTranslation(mayaVector, MSpace::kTransform);
				}

				newMeshFn.updateSurface();
				// Put mesh into the initial shading group
				MString cmd( "sets -e -fe initialShadingGroup " );
				cmd += newMeshFn.name();
				MGlobal::executeCommand( cmd );
				
				//Add to parent group ShatterMeshesGroup
				dagFn.addChild( objTransform );

			}

		}
		else
		{
			MGlobal::displayInfo("No triangles were selected!");
		}
	}
	else
	{
		MGlobal::displayInfo("No Meshes were selected! Nothing to do!");
	}
	
	setResult( "LarmorVoronoi command executed!\n" );
	std::cout.flush();

	return stat;
}

//build the MPointArray from MEL vector array
void getPointArrayFromMELvectorArray(MString arrayVectorName, MPointArray &pointsArray)
{
	//build MEL command:
	//
	//proc float[] _getArrayPointsByName() { 
	//	global vector $vpoints[]; 
	//	float $serialPoints[];
	//	for ($i = 0; $i < size($vpoints); $i++) { 
	//		vector $vpoint = $vpoints[$i]; 
	//		$serialPoints[size($serialPoints)] = $vpoint.x;
	//		$serialPoints[size($serialPoints)] = $vpoint.y;
	//		$serialPoints[size($serialPoints)] = $vpoint.z;
	//	}    
	//	return $serialPoints;
	//}
	//_getArrayPointsByName();
	//
	// Add point from MEL in this way:
	// global vector $vpoints[];
	// $vpoints = {};
	// for ($i = 0; $i <= 100; $i++) { 
	//       $vpoints[$i] = <<1.3, 2.4, 2.1+$i>>;  //Initialize point
	//       //print($vpoints[$i]);
	// } 
	// //print($vpoints);
	// LarmorVoronoi -p vpoints
	//

	MDoubleArray serializeDoubleArray;
	MString melCmd( "proc float[] _getArrayPointsByName() { " );
	melCmd += "global vector $"+arrayVectorName+"[]; ";
	melCmd += "float $serialPoints[]; ";
	melCmd += "for ($i = 0; $i < size($"+arrayVectorName+"); $i++) { ";
	melCmd += "vector $vpoint = $"+arrayVectorName+"[$i]; ";
	melCmd += "$serialPoints[size($serialPoints)] = $vpoint.x; ";
	melCmd += "$serialPoints[size($serialPoints)] = $vpoint.y; ";
	melCmd += "$serialPoints[size($serialPoints)] = $vpoint.z; ";
	melCmd += "} ";
	melCmd += "return $serialPoints; ";
	melCmd += "} _getArrayPointsByName();";
	
	//Execute the MEL command
	MGlobal::executeCommand( melCmd, serializeDoubleArray );

	//build MPointArray from serializeDoubleArray
	pointsArray.clear();
	for (int i = 0; i < serializeDoubleArray.length(); i += 3)
	{
		MPoint point(serializeDoubleArray[i], serializeDoubleArray[i+1], serializeDoubleArray[i+2]);
		pointsArray.append(point);
		std::cout << "Point X:" << point.x << ", ";
		std::cout << "Y:" << point.y << ", ";
		std::cout << "Z:" << point.z << std::endl;
	}

}

//Return the number of selected meshes
int buildMeshTrianglesFromSelection(std::list<Triangle> &meshTrianglesToShatter)
{
	
	// Get a list of currently selected objects
	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);

	MDagPath dagPath;
	MFnMesh meshFn;
	unsigned int meshesSelected = 0;

	// Iterate over the meshes
	MItSelectionList iter( selection, MFn::kMesh );
	for ( ; !iter.isDone(); iter.next() )
	{				
		meshesSelected++;
		iter.getDagPath(dagPath);
		meshFn.setObject(dagPath);

		MString meshMessageName("Mesh Selected: ");
		meshMessageName += meshFn.name();
		MGlobal::displayInfo(meshMessageName);

		//get points
		MPointArray meshPoints;
		meshFn.getPoints( meshPoints, MSpace::kWorld);
		//meshFn.getPoints( meshPoints, MSpace::kObject );

		//Get Mesh triangles
		MItMeshPolygon  itPolygon( dagPath, MObject::kNullObj );
		for (; !itPolygon.isDone(); itPolygon.next() )
		{
			// Get object-relative indices for the vertices in this face.
			MIntArray polygonVertices;
			itPolygon.getVertices( polygonVertices );

			// Get triangulation of this poly.
			int numTriangles = itPolygon.count();
			while ( numTriangles-- )
			{
				MPointArray nonTweaked;
				// object-relative vertex indices for each triangle
				MIntArray tv;
				// face-relative vertex indices for each triangle
				MIntArray localIndex;

				MStatus stat = itPolygon.getTriangle( numTriangles,
												nonTweaked,
												tv,
												MSpace::kObject );

				if ( stat == MS::kSuccess )
				{
					/*
					std::cout << "Triangle: " << std::endl;
					std::cout << "X0:" << meshPoints[tv[0]].x << std::endl;
					std::cout << "Y0:" << meshPoints[tv[0]].y << std::endl;
					std::cout << "Z0:" << meshPoints[tv[0]].z << std::endl;
					std::cout << "X1:" << meshPoints[tv[1]].x << std::endl;
					std::cout << "Y1:" << meshPoints[tv[1]].y << std::endl;
					std::cout << "Z1:" << meshPoints[tv[1]].z << std::endl;
					std::cout << "X2:" << meshPoints[tv[2]].x << std::endl;
					std::cout << "Y2:" << meshPoints[tv[2]].y << std::endl;
					std::cout << "Z2:" << meshPoints[tv[2]].z << std::endl;
					*/

					//Create the CGAL Points
					Point p1(meshPoints[tv[0]].x, meshPoints[tv[0]].y, meshPoints[tv[0]].z);
					Point p2(meshPoints[tv[1]].x, meshPoints[tv[1]].y, meshPoints[tv[1]].z);
					Point p3(meshPoints[tv[2]].x, meshPoints[tv[2]].y, meshPoints[tv[2]].z);

					//Add the triagle in meshTrianglesToShatter
					meshTrianglesToShatter.push_back(Triangle(p1,p2,p3));
				}
			}
		}
	}
	return meshesSelected;
}

void buildVerticesAndConnections(MeshData &meshData, 
	int &numTriagles, 
	MPointArray &pieceVertices, 
	MIntArray &pieceConnections,
	MIntArray &trianglePolyCounts,
	MIntArray &faceTypes)
{
	numTriagles = meshData.first.size();
	int numVertices = numTriagles * 3;

	pieceVertices.clear();
	pieceVertices.setLength( numVertices );
	pieceConnections.clear();
	pieceConnections.setLength( numVertices );
	trianglePolyCounts.clear();
	trianglePolyCounts.setLength( numTriagles );
	faceTypes.clear();
	faceTypes.setLength( numTriagles );

	MapPointMPoint mapPointMPoint;

	int numVertex = 0;
	int numTriangle = 0;

	TrianglesList::iterator triangleIter;
	TrianglesInfoList::iterator triangleInfoIter;
	for(triangleIter = meshData.first.begin(),
		triangleInfoIter = meshData.second.begin();
			triangleIter != meshData.first.end() &&
			triangleInfoIter != meshData.second.end();
			++triangleIter,
			++triangleInfoIter)
	{
		Triangle t = *triangleIter;
		TriangleInfo ti = *triangleInfoIter;
				
		for (int i = 0; i < 3; ++i)
		{
			Point tv = t.vertex(i);

			//Map used to not add different MPoint points for the same CGAL point
			MapPointMPoint::iterator foundPointMPoint;
			if ( (foundPointMPoint = mapPointMPoint.find(tv)) == mapPointMPoint.end())
			{
				//Point doesn't found in mapPointMPoint
				double tx = CGAL::to_double(tv.x());
				double ty = CGAL::to_double(tv.y());
				double tz = CGAL::to_double(tv.z());
				MPoint mayaPoint(tx, ty, tz);

				//Insert in mapPointMPoint
				mapPointMPoint.insert(PairPointMPoint(tv, mayaPoint));

				pieceVertices[numVertex] = mayaPoint;
				//pieceVertices.append(mayaPoint); //Don't use append if was used setLength 
			}
			else
			{
				//Point found in mapPointMPoint
				pieceVertices[numVertex] = foundPointMPoint->second;
				//pieceVertices.append(foundPointMPoint->second); //Don't use append if was used setLength 
			}

			pieceConnections[numVertex] = numVertex;
			//pieceConnections.append(numVertex); //Don't use append if was used setLength 
			numVertex++;
		}
		trianglePolyCounts[numTriangle] = 3;	
		
		if ((ti.cutType == CUTTERMESHER_TRIANGLE_NO_CUTTED) ||
				(ti.cutType == CUTTERMESHER_TRIANGLE_CUTTED))
		{
			//Triangle is not in cut plane
			faceTypes[numTriangle] = 0;
		}
		else if (ti.cutType == CUTTERMESHER_TRIANGLE_IN_CUT_PLANE)
		{
			//Triangle is in cut plane
			faceTypes[numTriangle] = 1;
		}
		else
		{
			MGlobal::displayInfo("ERROR: Something went wrong with TriangleInfo!");
			faceTypes[numTriangle] = 0;
		}

		numTriangle++;		
	}

}

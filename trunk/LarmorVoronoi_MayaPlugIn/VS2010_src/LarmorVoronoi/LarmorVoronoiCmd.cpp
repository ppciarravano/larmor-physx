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

#include "LarmorVoronoi.h"

// Use helper macro to register a command with Maya.
DeclareSimpleCommand( LarmorVoronoi, "Vers.0.1 (02/07/13) - Author: Pier Paolo Ciarravano www.larmor.com", "2012");

//
//	Description:
//		implements the MEL LarmorVoronoi command.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//      LarmorVoronoi [-np NUM_SHATTER_POINTS] [-d true|false]
//      NUM_SHATTER_POINTS is the number of the voronoi cells (default 10).
//      If  -d true  the shatter separates the disjointed surfaces (default false).
//
MStatus LarmorVoronoi::doIt( const MArgList& args )
{
	MGlobal::displayInfo("Start LarmorVoronoi...");
	MGlobal::displayInfo("Use: LarmorVoronoi -np int_num_pieces -d bool_disjoint");

	//MStatus stat = MS::kSuccess;
	MStatus stat;

	//Read the command arguments
	int numberPieces = 10;
	bool disjointMeshes = false;
	unsigned int index;
	index = args.flagIndex( "np", "npieces" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, numberPieces );
	}
	index = args.flagIndex( "d", "disjoint" );
	if( index != MArgList::kInvalidArgIndex )
	{
		args.get( index+1, disjointMeshes );
	}

	if (numberPieces < 2)
	{
		setResult( "Minimum number of pieces is 2!\n" );
		return stat;
	}
	MString argsMessage("Execute Voronoi Shatter with number pieces: ");
	argsMessage += (int)numberPieces;
	argsMessage += " and disjoint meshes:";
	argsMessage += (bool)disjointMeshes;
	MGlobal::displayInfo(argsMessage);
	
	//Store the mesh triangles for Larmor Voronoi shatter
	std::list<Triangle> meshTrianglesToShatter;
	
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

				stat = itPolygon.getTriangle( numTriangles,
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

			//Run Voronoi shatter
			MGlobal::displayInfo("Running Voronoi statter...");
			//Do Voronoi shatter
			std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(meshTrianglesToShatter, numberPieces, disjointMeshes);
			//Center meshes in their barycenters
			std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);
			MGlobal::displayInfo("Voronoi statter complete");

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

				//CGAL Point of translation for the barycenters
				Point originObj = meshDataPartTranslated.second;
				double cm_x = CGAL::to_double(originObj.x());
				double cm_y = CGAL::to_double(originObj.y());
				double cm_z = CGAL::to_double(originObj.z());
				MVector mayaVector(cm_x, cm_y, cm_z);
				//Translate the mesh in the proper position
				MFnTransform objMFnTransform(objTransform);
				objMFnTransform.setTranslation(mayaVector, MSpace::kTransform);

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
	
	return stat;
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
			double tx = CGAL::to_double(tv.x());
			double ty = CGAL::to_double(tv.y());
			double tz = CGAL::to_double(tv.z());
			MPoint mayaPoint(tx, ty, tz);
			pieceVertices[numVertex] = mayaPoint;
			//pieceVertices.append(mayaPoint);
			pieceConnections[numVertex] = numVertex;
			//pieceConnections.append(numVertex);
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

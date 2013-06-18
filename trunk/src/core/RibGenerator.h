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

#ifndef RIBGENERATOR_H_
#define RIBGENERATOR_H_

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

#include "../shatter/MeshViewer.h"
#include "../shatter/CutterMesher.h"
#include "../shatter/Custom.h"
#include "../shatter/util.h"
#include "../shatter/DelaunayVoronoi.h"

#include "core_classes.h"

namespace LarmorPhysx
{

	typedef boost::unordered_map<PairUnsignedInt, DynamicObject> MapOrderedPairDynamicObject;
	typedef std::pair<PairUnsignedInt, DynamicObject> PairDynamicObject;

	void generateRIB();
	void generateRIBScene(unsigned int idFrame, DynamicObjectVector &dynamicObjects);
	void generateRIBSceneMotionBlur(unsigned int idFrame, DynamicObjectVector &dynamicObjects,
			MapOrderedPairDynamicObject &mapIdFrameIdObjectToDynamicObjects);
	void generateRIBFrame(unsigned int idFrame, unsigned int diplayImageNumber);
	void generateRIBCamera(unsigned int idFrame);
	void generateRIBMesh(StaticObject &staticObject);

}

#endif /* RIBGENERATOR_H_ */

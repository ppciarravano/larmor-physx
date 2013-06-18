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

#ifndef SCENES_CREATION_H_
#define SCENES_CREATION_H_

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

	void generateExplodingScene();

	// Demo scenes
	void createFirstFrame_Scene1();
	void createFirstFrame_Scene2();
	void createFirstFrame_Scene3();
	void createFirstFrame_Scene4();
	void createFirstFrame_Scene5();
	void createFirstFrame_Scene6();
	void createFirstFrame_Scene7();
	void createFirstFrame_Scene8();
	void createFirstFrame_Scene9();
	void createFirstFrame_Scene10();
	void createFirstFrame_Scene11();

}

#endif /* SCENES_CREATION_H_ */

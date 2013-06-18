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

#ifndef ANIMATION_VIEWER_H_
#define ANIMATION_VIEWER_H_

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

#include "windows.h"
#include "gl/gl.h"
#include "gl/glu.h"
//#define FREEGLUT_STATIC
#include "GL/freeglut.h"

using namespace std;

#include "../shatter/MeshViewer.h"
#include "../shatter/CutterMesher.h"
#include "../shatter/Custom.h"
#include "../shatter/util.h"
#include "../shatter/DelaunayVoronoi.h"

#include "core_classes.h"

using namespace LarmorPhysx;

namespace LarmorPhysxViewer
{

	void run_animation_viewer();

}

#endif /* ANIMATION_VIEWER_H_ */

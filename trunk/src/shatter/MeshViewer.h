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

#ifndef MESHVIEWER_H_
#define MESHVIEWER_H_

#include "Custom.h"

void openMeshViewer(VectListCTriangle3d input, VectListCSegment3d inputSegments);

namespace MeshViewerAnim
{
	void openMeshViewerAnim(VectListCTriangle3d input, VectCPoint3d points);
}

#endif /* MESHVIEWER_H_ */

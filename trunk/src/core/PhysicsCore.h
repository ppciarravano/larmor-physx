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

#ifndef PHYSICSCORE_H_
#define PHYSICSCORE_H_

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

/*
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btDefaultMotionState.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btDefaultMotionState.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
*/
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include <boost/thread.hpp>

#include "../shatter/MeshViewer.h"
#include "../shatter/CutterMesher.h"
#include "../shatter/Custom.h"
#include "../shatter/util.h"
#include "../shatter/DelaunayVoronoi.h"

#include "core_classes.h"

namespace LarmorPhysx
{


	class PhysicsCore
	{
		protected:

			//btDynamicsWorld class member
			btDynamicsWorld* m_dynamicsWorld;

			//Frame containers
			Frame stepFrame;
			unsigned int idFrame;

			DynamicObjectPtrVector dynamicObjectsInWorld;

			unsigned int idGroupCounter; //last greater idGroup add in the scene, used to assign new group to the voronoi pieces

			double timePosition;

			MapOrderedPairCollisionPoint maxMapCollisionPointPair;
			MapObjectIdCollisionPoint maxMapCollisionPoint;

		public:

			PhysicsCore();
			~PhysicsCore();

			void initPhysics();
			void initWorld();
			void stepWorld();
			void renderWorld();
			void updateWorld();
			void applyExternalForces();

			void destroyWorld();
			void loadStartFrame(int numFrame);
			void addDynamicObjectsToWorld();
			void copyDynamicObjectsInWorldIntoFrame();

	};

	inline btScalar	calculateCombinedFriction(float friction0, float friction1);
	inline btScalar	calculateCombinedRestitution(float restitution0, float restitution1);
	bool customMaterialCombinerCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);
	void preTickCallbackLog(btDynamicsWorld *world, btScalar timeStep);

}

#endif /* PHYSICSCORE_H_ */

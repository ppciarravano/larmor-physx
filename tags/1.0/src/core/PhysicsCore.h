/*****************************************************************************
 * Larmor-Physx Version 1.0 2013
 * Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of Larmor-Physx (http://code.google.com/p/larmor-physx/).
 *
 * Larmor-Physx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Larmor-Physx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Larmor-Physx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 * $Id$
 *
 ****************************************************************************/

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

#define BT_USE_DOUBLE_PRECISION 1
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

			static std::list<TranslatedMeshData> shatterObject(DynamicObject* dynamicObject, CollisionPoint& cpMax);

	};

	inline btScalar	calculateCombinedFriction(float friction0, float friction1);
	inline btScalar	calculateCombinedRestitution(float restitution0, float restitution1);
	bool customMaterialCombinerCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);
	void preTickCallbackLog(btDynamicsWorld *world, btScalar timeStep);

}

#endif /* PHYSICSCORE_H_ */

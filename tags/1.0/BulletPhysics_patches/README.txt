/*****************************************************************************
 * Larmor-Physx Version 1.0 2014
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
 
Bullet Physics files patches for GImpactCollision to apply to Bullet Physics Vers.2.81-rev2613 (LarmorPhysx RevanoVX Revision: r30 13 Sept. 2014)

If you will apply this patches then you can use -DBULLET_USE_CGAL_PATCH=1 in LarmorPhysx RevanoVX (configure this in CMakeLists.txt or use #define BULLET_USE_CGAL_PATCH 1)

To apply the patches:
Edit the file BulletPhysics_patches/CMakeLists.txt and update INCLUDE_DIRECTORIES and LINK_DIRECTORIES with the correct paths of your third-party libraries.
Edit the file BulletPhysics_patches/patch_BulletCollision_CMakeLists.txt and update TARGET_LINK_LIBRARIES with your third-party library names.

apply the patch files:
patch bullet-2.81-rev2613/CMakeLists.txt < ../larmor-physx/BulletPhysics_patches/patch_CMakeLists.txt
patch bullet-2.81-rev2613/src/BulletCollision/CMakeLists.txt < ../larmor-physx/BulletPhysics_patches/patch_BulletCollision_CMakeLists.txt
patch bullet-2.81-rev2613/src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp < ../larmor-physx/BulletPhysics_patches/patch_btGImpactCollisionAlgorithm.cpp 
patch bullet-2.81-rev2613/src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h < ../larmor-physx/BulletPhysics_patches/patch_btGImpactCollisionAlgorithm.h



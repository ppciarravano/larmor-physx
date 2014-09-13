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

Run instructions for LarmorPhysx RevanoVX (standalone) Revision: r30 13 Sept. 2014.

The compiled version of source code r28 RevanoVX.exe is for Windows 64bit.

Steps to run a test scene:

1) create an empty directory: e.g. dir_test

2) copy in dir_test: RevanoVX.exe or the compiled Linux version you built

3) for Windows if you didn't compile by yourself: unzip the file dependent_dlls.zip in the dir_test

4) copy the file larmorphysx_config.ini in dir_test

5) copy the content of meshes/ in dir_test

6) cd dir_test

7) mkdir scene_test_dir

8) create example scene:
./RevanoVX --action createscene16

9) run the simulation:
./RevanoVX --action simulation

10) view animation:
./RevanoVX --action animationviewer

press F11 to play the animation and drag the mouse to change the view point.
Have a look to the source code for all the commands and scene examples.

Thanks!












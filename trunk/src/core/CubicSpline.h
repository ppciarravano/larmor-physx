/*****************************************************************************
 * Larmor-Physx Version 1.1 2013-2014
 * Copyright (c) 2013-2014 Pier Paolo Ciarravano - http://www.larmor.com
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

#ifndef CUBIC_SPLINE_H_
#define CUBIC_SPLINE_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <math.h>

using namespace std;

#include "../shatter/CutterMesher.h"
#include "../shatter/Custom.h"
#include "../shatter/util.h"
#include "../shatter/DelaunayVoronoi.h"

#include "core_classes.h"

using namespace LarmorPhysx;

namespace LarmorPhysx
{

	//void test_cubic_spline();

	class Cubic {

			private:
				double a;
				double b;
				double c;
				double d;

			public:
				Cubic(double ap, double bp, double cp, double dp) : a(ap), b(bp), c(cp), d(dp) {}

				double eval(double u) {
					return (((d*u) + c)*u + b)*u + a;
				}

	};


	class CubicSpline {

		private:
			vector<LVector3> points;
			vector<Cubic> xCubics;
			vector<Cubic> yCubics;
			vector<Cubic> zCubics;

			void calcNaturalCubic(double LVector3::*ptrProp, vector<Cubic> *cubics);

		public:

			CubicSpline() {}

			void addPoint(LVector3 point);
			vector<LVector3> getPoints();
			LVector3 getPoint(int cubicNum, double cubicPos);
			LVector3 getPoint(double position);
			void compute();

	};


}

#endif /* CUBIC_SPLINE_H_ */

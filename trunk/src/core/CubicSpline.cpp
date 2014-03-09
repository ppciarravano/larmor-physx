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

//Implements natural cubic splines based on:
//http://www.java-gaming.org/topics/2d-amp-3d-spline-think-smooth-curve-through-n-points-function/9830/view.html
//http://www.cse.unsw.edu.au/~lambert/splines/

#include "CubicSpline.h"

namespace LarmorPhysx
{

	/*
	void test_cubic_spline()
	{
		std::cout << "test_cubic_spline: " << std::endl;

		//Test member pointer
		double LVector3::*ptrPropX = &LVector3::x;
		double LVector3::*ptrPropY = &LVector3::y;
		double LVector3::*ptrPropZ = &LVector3::z;
		LVector3 testval(2.3, 4.5, 8.6);
		cout << "testval: " << testval.*ptrPropX << ", " << testval.*ptrPropY << ", " << testval.*ptrPropZ << endl;

		CubicSpline spline;
		spline.addPoint(LVector3(5.0f, 7.0f, 8.0f));
		spline.addPoint(LVector3(2.0f, 11.0f, 6.0f));
		spline.addPoint(LVector3(9.0f, 4.0f, 3.0f));
		spline.compute();

		for(int i = 0; i <= 100; i++)
		{
			LVector3 p = spline.getPoint(0.01 * i);
			cout << "Spline: [" << i << "] = " << p.x << ", " << p.y << ", " << p.z << endl;
		}

		LVector3 p0 = spline.getPoint(0, 0.0);
		cout << "Spline: [p0] = " << p0.x << ", " << p0.y << ", " << p0.z << endl;
		LVector3 p1 = spline.getPoint(1, 0.0);
		cout << "Spline: [p1] = " << p1.x << ", " << p1.y << ", " << p1.z << endl;
		LVector3 pN = spline.getPoint(2, 1.0);
		cout << "Spline: [pN] = " << pN.x << ", " << pN.y << ", " << pN.z << endl;
	}
	*/

	void CubicSpline::addPoint(LVector3 point)
	{
		points.push_back(point);
	}

	vector<LVector3> CubicSpline::getPoints()
	{
		return points;
	}

	LVector3 CubicSpline::getPoint(int cubicNum, double cubicPos)
	{
		if (cubicNum >= xCubics.size())
		{
			cubicNum = xCubics.size() - 1;
			cubicPos = 1.0L;
		}

		LVector3 point(xCubics.at(cubicNum).eval(cubicPos),
						yCubics.at(cubicNum).eval(cubicPos),
						zCubics.at(cubicNum).eval(cubicPos));

		return point;
	}

	LVector3 CubicSpline::getPoint(double position)
	{
		position = position * xCubics.size();
		int cubicNum = floor(position);
		double cubicPos = (position - cubicNum);

		if (cubicNum >= xCubics.size())
		{
			cubicNum = xCubics.size() - 1;
			cubicPos = 1.0L;
		}

		LVector3 point(xCubics.at(cubicNum).eval(cubicPos),
						yCubics.at(cubicNum).eval(cubicPos),
						zCubics.at(cubicNum).eval(cubicPos));

		return point;
	}

	void CubicSpline::calcNaturalCubic(double LVector3::*ptrProp, vector<Cubic>* cubics)
	{
		int num = points.size() - 1;

		double* gamma = new double[num+1];
		double* delta = new double[num+1];
		double* D = new double[num+1];

		gamma[0] = 1.0L / 2.0L;
		for(int i = 1; i < num; i++)
		{
			gamma[i] = 1.0L /(4.0L - gamma[i-1]);
		}
		gamma[num] = 1.0L / (2.0L - gamma[num-1]);

		double p0 = points.at(0).*ptrProp;
		double p1 = points.at(1).*ptrProp;

		delta[0] = 3.0L * (p1 - p0) * gamma[0];
		for(int i = 1; i < num; i++)
		{
			p0 = points.at(i - 1).*ptrProp;
			p1 = points.at(i + 1).*ptrProp;
			delta[i] = (3.0L * (p1 - p0) - delta[i - 1]) * gamma[i];
		}
		p0 = points.at(num - 1).*ptrProp;
		p1 = points.at(num).*ptrProp;

		delta[num] = (3.0f * (p1 - p0) - delta[num - 1]) * gamma[num];

		D[num] = delta[num];
		for(int i = num - 1; i >= 0; i--)
		{
			D[i] = delta[i] - gamma[i] * D[i+1];
		}

		cubics->clear();
		for(int i = 0; i < num; i++)
		{
			p0 = points.at(i).*ptrProp;
			p1 = points.at(i + 1).*ptrProp;
			Cubic cubic(
					p0,
					D[i],
					3*(p1 - p0) - 2*D[i] - D[i+1],
					2*(p0 - p1) +   D[i] + D[i+1]
            );
			cubics->push_back(cubic);
		}

		//Delete pointers
		delete[] gamma;
		delete[] delta;
		delete[] D;

	}

	void CubicSpline::compute()
	{
		calcNaturalCubic(&LVector3::x, &xCubics);
		calcNaturalCubic(&LVector3::y, &yCubics);
		calcNaturalCubic(&LVector3::z, &zCubics);
	}

}


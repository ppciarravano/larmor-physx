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

/*
 * MeshMassProperties.cpp
 *
 * Code extract from:
 * "Fast and Accurate Computation of Polyhedral Mass Properties" by Brian Mirtich,
 *  journal of graphics tools, volume 1, number 2, 1996.
 *  http://www.cs.berkeley.edu/~jfc/mirtich/massProps.html
 * and
 * "Polyhedral Mass Properties (Revisited)" by David Eberly
 *  http://www.geometrictools.com/Documentation/PolyhedralMassProperties.pdf
 */

#include "DelaunayVoronoi.h"

#include <iostream>
#include <math.h>

typedef double REALD;

inline void subexpressions_integral_terms(REALD w0, REALD w1, REALD w2, REALD &f1, REALD &f2, REALD &f3, REALD &g0, REALD &g1, REALD &g2)
{
	REALD temp0 = w0+w1;
	REALD temp1 = w0*w0;
	REALD temp2 = temp1+w1*temp0;
	f1 = temp0+w2;
	f2 = temp2+w2*f1;
	f3 = w0*temp1+w1*temp2+w2*f2;
	g0 = f2+w0*(f1+w0);
	g1 = f2+w1*(f1+w1);
	g2 = f2+w2*(f1+w2);
}

//The return values are the mass, the center of mass, and the inertia tensor relative to the
// center of mass. The code assumes that the rigid body has constant density 1. If the rigid body has constant
// density D, then you need to multiply the output mass by D and the output inertia tensor by D.
void calculateMassCenterInertia(TrianglesList &triangles, double &massReturn, PointCGAL &cmReturn, Vector &inertiaReturn)
{
	const REALD mult[10] = {1./6., 1./24., 1./24., 1./24., 1./60., 1./60., 1/60., 1./120., 1./120., 1./120.};
	REALD intg[10] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}; // order: 1, x, y, z, x^2, y^2, z^2, xy, yz, zx

	std::list<Triangle>::iterator triangleIter;
	for(triangleIter = triangles.begin(); triangleIter != triangles.end(); ++triangleIter)
	{
		Triangle t = *triangleIter;

		REALD x0 = CGAL::to_double(t.vertex(0).x());
		REALD y0 = CGAL::to_double(t.vertex(0).y());
		REALD z0 = CGAL::to_double(t.vertex(0).z());
		REALD x1 = CGAL::to_double(t.vertex(1).x());
		REALD y1 = CGAL::to_double(t.vertex(1).y());
		REALD z1 = CGAL::to_double(t.vertex(1).z());
		REALD x2 = CGAL::to_double(t.vertex(2).x());
		REALD y2 = CGAL::to_double(t.vertex(2).y());
		REALD z2 = CGAL::to_double(t.vertex(2).z());

		// get edges and cross product of edges
		REALD a1 = x1-x0;
		REALD b1 = y1-y0;
		REALD c1 = z1-z0;
		REALD a2 = x2-x0;
		REALD b2 = y2-y0;
		REALD c2 = z2-z0;
		REALD d0 = b1*c2-b2*c1;
		REALD d1 = a2*c1-a1*c2;
		REALD d2 = a1*b2-a2*b1;

		// compute integral terms
		REALD f1x, f2x, f3x, g0x, g1x, g2x, f1y, f2y, f3y, g0y, g1y, g2y, f1z, f2z, f3z, g0z, g1z, g2z;
		subexpressions_integral_terms(x0, x1, x2, f1x, f2x, f3x, g0x, g1x, g2x);
		subexpressions_integral_terms(y0, y1, y2, f1y, f2y, f3y, g0y, g1y, g2y);
		subexpressions_integral_terms(z0, z1, z2, f1z, f2z, f3z, g0z, g1z, g2z);

		// update integrals
		intg[0] += d0*f1x;
		intg[1] += d0*f2x;
		intg[2] += d1*f2y;
		intg[3] += d2*f2z;
		intg[4] += d0*f3x;
		intg[5] += d1*f3y;
		intg[6] += d2*f3z;
		intg[7] += d0*(y0*g0x+y1*g1x+y2*g2x);
		intg[8] += d1*(z0*g0y+z1*g1y+z2*g2y);
		intg[9] += d2*(x0*g0z+x1*g1z+x2*g2z);

	}

	//Multiple the coefficients
	for (int i = 0; i < 10; i++)
	{
		intg[i] *= mult[i];
	}

	//Mass of the body with a constant density 1
	REALD mass = intg[0];

	//Center of mass
	REALD cm_x = intg[1]/mass;
	REALD cm_y = intg[2]/mass;
	REALD cm_z = intg[3]/mass;

	//Inertia tensor relative to center of mass
	REALD inertia_xx = intg[5]+intg[6]-mass*(cm_y*cm_y+cm_z*cm_z);
	REALD inertia_yy = intg[4]+intg[6]-mass*(cm_z*cm_z+cm_x*cm_x);
	REALD inertia_zz = intg[4]+intg[5]-mass*(cm_x*cm_x+cm_y*cm_y);
	//REALD inertia_xy = -(intg[7]-mass*cm_x*cm_y);
	//REALD inertia_yz = -(intg[8]-mass*cm_y*cm_z);
	//REALD inertia_xz = -(intg[9]-mass*cm_z*cm_x);

	//Return the references values
	massReturn = fabs(mass);
	cmReturn = PointCGAL(cm_x, cm_y, cm_z);
	inertiaReturn = Vector(inertia_xx, inertia_yy, inertia_zz);

}


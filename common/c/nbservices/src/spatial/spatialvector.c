/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * vector.c: created 2003/03/17 by Mark Goddard.
 *
 * Copyright (c) 2004 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 *
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "palmath.h"
#include "paldebuglog.h"
#include "spatialvector.h"
#include "transformutility.h"


// Local Functions ...................................................................................................

static void ProjectPolygonToLine(const NB_Vector* line,
                                 const NB_Vector* polygon,
                                 int count,
                                 double* min,
                                 double* max);

static void GetPolygonEdge(const NB_Vector* polygon,
                           int count,
                           int i,
                           double* x1,
                           double* y1,
                           double* x2,
                           double* y2);

static boolean ProjectPolygon(const NB_Vector* polygon1,
                             int count1,
                             boolean isRectangle1,
                             const NB_Vector* polygon2,
                             int count2,
                             boolean isRectangle2);


// Public Functions ..................................................................................................

/*! Simple setter function */
void        
vector_set(NB_Vector* output, double x, double y, double z)
{
    output->x = x;
    output->y = y;
    output->z = z;
}

double	
vector_magnitude(const NB_Vector* px)
{
	return nsl_sqrt(px->x *px->x + px->y * px->y + px->z * px->z);
}

void 
vector_cross_product(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py)
{
	pout->x = (px->y*py->z - px->z*py->y);
	pout->y = (px->z*py->x - px->x*py->z);
	pout->z = (px->x*py->y - px->y*py->x);
}

double 
vector_dot_product(const NB_Vector* px, const NB_Vector* py)
{
	return px->x * py->x + px->y * py->y + px->z * py->z;
}

void		
vector_from_lat_lon(NB_Vector* pout, double lat, double lon)
{
	double latr = TO_RAD(lat);
	double lonr = TO_RAD(lon);
	double sinlatr = nsl_sin(latr);
	double coslatr = nsl_cos(latr);
	double sinlonr = nsl_sin(lonr);
	double coslonr = nsl_cos(lonr);

	pout->z = sinlatr * RADIUS_EARTH_METERS;
	pout->x = coslatr * coslonr * RADIUS_EARTH_METERS;
	pout->y = coslatr * sinlonr * RADIUS_EARTH_METERS;
}

void
vector_to_lat_lon(double* plat, double* plon, const NB_Vector* pv)
{
	double at;

	*plat = TO_DEG(PI_OVER_TWO - nsl_acos(pv->z / RADIUS_EARTH_METERS));
	
	if (pv->x == 0.0) {

		if (pv->y > 0.0)
			*plon = 90.0;
		else
			*plon = -90.0;
	}
	else {
		
		at = nsl_atan(pv->y / pv->x);

		if (pv->x > 0.0)
			*plon = TO_DEG(at);
		else if (pv->y > 0.0)
			*plon = TO_DEG(PI + at);
		else 
			*plon = TO_DEG(at - PI);
	}
}

void		
vector_normalize(NB_Vector* pout, const NB_Vector* pv)
{
	double length = vector_magnitude(pv);

	pout->x = pv->x / length;
	pout->y = pv->y / length;
	pout->z = pv->z / length;
}

void
vector_normalize_to_length(NB_Vector* pout, const NB_Vector* pv, double length_out)
{
	double scale = vector_magnitude(pv) / length_out;

	pout->x = pv->x / scale;
	pout->y = pv->y / scale;
	pout->z = pv->z / scale;
}

void		
vector_multiply(NB_Vector* pout, const NB_Vector* pv, double v)
{
	pout->x = pv->x * v;
	pout->y = pv->y * v;
	pout->z = pv->z * v;
}

void		
vector_subtract(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py)
{
	pout->x = px->x - py->x;
	pout->y = px->y - py->y;
	pout->z = px->z - py->z;
}

void
vector_add(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py)
{
	pout->x = px->x + py->x;
	pout->y = px->y + py->y;
	pout->z = px->z + py->z;
}

double	
vector_distance(const NB_Vector* px, const NB_Vector* py)
{
	NB_Vector x;
	NB_Vector y;
	double dp;

 	vector_normalize(&x, px);
	vector_normalize(&y, py);

	dp = vector_dot_product(&x,&y);

	// workaround for imprecise math 
	if (dp > 1.0)
		dp = 1.0;

	return nsl_acos(dp) * RADIUS_EARTH_METERS;
}

boolean	
vector_is_point_on_line(const NB_Vector* pp, const NB_Vector* pa, const NB_Vector* pb)
{
	NB_Vector cp1;
	NB_Vector cp2;
	double vdp;
	
	vector_cross_product(&cp1, pa, pp);
	vector_cross_product(&cp2, pb, pp);
	vdp = vector_dot_product(&cp1, &cp2);

	return ((vdp < 0) ? TRUE : FALSE);
}

int	
project_point_to_line(double p_lat, double p_lon, double l1_lat, double l1_lon, double l2_lat, double l2_lon, double* p_proj_lat, double* p_proj_lon, double* pdist)
{
	/* First step is to convert lat/lon coordinates into x,y,z vectors */

	NB_Vector vp;
	NB_Vector vla;
	NB_Vector vlb;
	NB_Vector xproduct;
	NB_Vector vn;
    double pn;
    NB_Vector vpn;
    NB_Vector C;
    NB_Vector S;
	double d1, d2;
	double coeff;

	coeff = 0.0;

	vector_from_lat_lon(&vp, p_lat, p_lon);
	vector_from_lat_lon(&vla, l1_lat, l1_lon);
	vector_from_lat_lon(&vlb, l2_lat, l2_lon);

	/* Next, we evaluate the length-normalized vector of (VLA x VLB).	Result is vn */

	vector_cross_product(&xproduct, &vla, &vlb);

	vector_normalize(&vn, &xproduct);

	/* Next, we evaluate the expression C = VP - (VP . vn) * vn.  C is the closest point on the plane */
    pn = vector_dot_product(&vp, &vn);

    vector_multiply(&vpn, &vn, pn);
    vector_subtract(&C, &vp, &vpn);

	/* Send a ray from the earth center through C to the earth's surface to get the solution point, S.  Formula is S = C / |C| * r */

	vector_normalize(&C, &C);
	vector_multiply(&S, &C, RADIUS_EARTH_METERS);

	vector_to_lat_lon(p_proj_lat, p_proj_lon, &S);

	/* Check to see if solution point is actually within the line segment. If so, we just compute distance and return */

	if (vector_is_point_on_line(&S, &vla, &vlb)) {
		*pdist = vector_distance(&vp, &S);
		return 0;
	}
	else {
		/* Otherwise, point is "out of bounds", so we must compute distance from vla & vlb, and return shortest */
		d1 = vector_distance(&vp, &vla);
		d2 = vector_distance(&vp, &vlb);

		if (d1 < d2) {
			*pdist = d1;
			*p_proj_lat = l1_lat;
			*p_proj_lon = l1_lon;
			return -1;
		}
		else {
			*pdist = d2;
			*p_proj_lat = l2_lat;
			*p_proj_lon = l2_lon;
			return 1;
		}
	}
}

/* See header for description */
boolean
vector_is_point_in_polygon(const NB_Vector* point, const NB_Vector* polygon, int pointsInPolygon)
{
    /*
        This function only works for convex polygons. 

        If the polygon is convex then we can just walk the path of the polygon and if the point is 
        always on one side (based on if points are in clockwise or counter-clockwise order) then the point 
        is inside the polygon.
    */
    int i = 0;
    double x = 0;
    double y = 0;
    double multiplier = 1.0;

    // The polygon has to have at least 3 points (otherwise it would not be a polygon)
    if (!point)
    {
        return FALSE;
    }

    // Point to compare
    x = point->x;
    y = point->y;

    // This multiplier becomes negative for clockwise polygons. See below.

    // The polygon has to have at least 3 points (otherwise it would not be a polygon)
    if (!polygon || (pointsInPolygon <= 2))
    {
        return FALSE;
    }
    else
    /*
        Check if the polygon is in clockwise or counter-clockwise order. We determine this by the calculating the cross-product
        of any adjacent edges. If the cross product is negative then it is clockwise, otherwise counter-clockwise ordering.
    */
    {
        NB_Vector out = {0};
        NB_Vector edge1 = {0};
        NB_Vector edge2 = {0};

        // Get the first two edges
        vector_subtract(&edge1, &polygon[1], &polygon[0]);
        vector_subtract(&edge2, &polygon[2], &polygon[1]);

        vector_cross_product(&out, &edge1, &edge2);
        
        // We only care for the z-value, if it is negative then we have clockwise ordering
        if (out.z < 0)
        {
            multiplier = -1.0;
        }
    }

    // For all path in the polygon 
    for (i = 0; i < pointsInPolygon; ++i)
    {
        double value = 0;
        double x1 = 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = 0;

        // Get start/end of path
        GetPolygonEdge(polygon, pointsInPolygon, i, &x1, &y1, &x2, &y2);

        // If this equation is smaller then zero then the point is to the left of the path (for clockwise) and to the 
        // right (for counter-clockwise). That means the point is not inside the polygon.
        value = (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
        if ((value * multiplier) < 0)
        {
            return FALSE;
        }
    }

    // The point must be inside the polygon
    return TRUE;
}

/* See header for description */
boolean
vector_polygons_intersect(const NB_Vector* polygon1,
                          int count1,
                          boolean isRectangle1,
                          const NB_Vector* polygon2,
                          int count2,
                          boolean isRectangle2)
{
    /*
        Calculating the intersection of polygons is not as trivial at is might seem. There are different approaches for this
        problem. If the polygons are convex then it is easier to compute. 
        We could call the sutherland-hodgman function to clip the polygons on all its edges. If all clipped polygons are
        empty then the polygons do not intersect. 

        The approach that is used here is the 'SAT' (Separating Axis Theorem) polygon collision algorithm. This is an 
        "early out" algorithm which is very fast for non-colliding/intersecting polygons. Polygons are considered non-intersecting
        if a line can be found between the polygons. To do this we check all normals of all edges of both polygons and project
        all points to the normals. If a gap is found then the polygons don't intersect. Search online for 
        "(Separating Axis Theorem) polygon collision algorithm" to learn more.
    */

    // Project all points to polygon1
    if (ProjectPolygon(polygon1, count1, isRectangle1, polygon2, count2, isRectangle2) == FALSE)
    {
        return FALSE;
    }

    // Project all points to polygon2
    if (ProjectPolygon(polygon2, count2, isRectangle2, polygon1, count1, isRectangle1) == FALSE)
    {
        return FALSE;
    }

    // Polygons intersect
    return TRUE;
}

void
calculate_segment_normals(NB_Vector* p1, NB_Vector* p2, double normal_length,
    NB_Vector* n1l, NB_Vector* n1r, NB_Vector* n2l, NB_Vector* n2r, 
    double* plengthremain, NB_Vector* last_pt, NB_Vector* last_dir,
    double alt_normal_length, NB_Vector* alt_n1l, NB_Vector* alt_n1r, NB_Vector* alt_n2l, NB_Vector* alt_n2r)
{
    NB_Vector delta = {0};
    NB_Vector delta_out = {0};
    NB_Vector normal = {0};
    NB_Vector normal2 = {0};
    NB_Vector unit_z = {0};
    NB_Vector p2_new = {0};

    NB_Vector alt_normal2 = {0};

    double length = 0;

    unit_z.z = 1.0;

    vector_subtract(&delta, p2, p1);

    if (plengthremain != NULL)
        length = vector_magnitude(&delta);

    vector_cross_product(&normal, &delta, &unit_z);

    vector_normalize_to_length(&normal2, &normal, normal_length);
    if (alt_normal_length != 0)
        vector_normalize_to_length(&alt_normal2, &normal, alt_normal_length);

    if (n1l)
        vector_add(n1l, p1, &normal2);

    if (n1r)
        vector_subtract(n1r, p1, &normal2);

    if (alt_n1l)
        vector_add(alt_n1l, p1, &alt_normal2);

    if (alt_n1r)
        vector_subtract(alt_n1r, p1, &alt_normal2);

    if (plengthremain != NULL && length > *plengthremain) {

        // need to move the second point to somewhere along the line
        vector_normalize_to_length(&delta, &delta, *plengthremain);
        vector_add(&p2_new, p1, &delta);
        *plengthremain = 0;
    }
    else {

        p2_new = *p2;
    }

    if (n2l)
        vector_add(n2l, &p2_new, &normal2);

    if (n2r)
        vector_subtract(n2r, &p2_new, &normal2);

    if (alt_n2l)
        vector_add(alt_n2l, &p2_new, &alt_normal2);

    if (alt_n2r)
        vector_subtract(alt_n2r, &p2_new, &alt_normal2);

    if (last_pt)
        *last_pt = p2_new;

    vector_normalize(&delta_out, &delta);
    if (last_dir)
        *last_dir = delta_out;

    if (plengthremain != NULL)
        (*plengthremain) -= length;
}


static int
output_point(NB_Vector* p, NB_Vector* pout, int* poutlen, int outsize)
{
	if (*poutlen >= outsize)
		return -1;

	pout[(*poutlen)++] = *p;

	return 0;
}

/* Taken from page 128 of Foley/vanDam */
int 
sutherland_hodgman_polygon_clip(NB_Vector* pin, int in_len, NB_Vector* pout, int out_size,
								clippointfunc clipper, intersectfunc intersect, void* userdata)
{
	NB_Vector* s =NULL;
	NB_Vector* p = NULL;
	NB_Vector i ={0};

	int outlen = 0;
	int j;

	/* Must have a clipping function */
	if (clipper == NULL || intersect == NULL)
		goto error;

	s = &pin[in_len-1];

	for (j=0; j<in_len;j++) {

		p = pin + j; /* Select the next vertex */

		if (clipper(p, userdata)) {

			if (clipper(s, userdata)) {

				/* Case 1 */
				if (pout == NULL)
					outlen++;
				else if (output_point(p, pout, &outlen, out_size) < 0)
					goto error;
			}
			else {

				/* Case 4 */
				intersect(s, p, &i, userdata);

				if (pout == NULL)
					outlen++;
				else if (output_point(&i, pout, &outlen, out_size) < 0)
					goto error;

				if (pout == NULL)
					outlen++;
				else if (output_point(p, pout, &outlen, out_size) < 0)
					goto error;
			}
		}
		else {

			if (clipper(s, userdata)) {

				/* Case 2 */
				intersect(s, p, &i, userdata);

				if (pout == NULL)
					outlen++;
				else if (output_point(&i, pout, &outlen, out_size) < 0)
					goto error;
			}
			else {

				/* Case 3 */
				/* No Action */
			}
		}

		s = p;
	}

	return outlen;

error:
	/* An error occurred -- Invalid clip func, or no space for output */
	return -1;
}

boolean 
zmin_clip(NB_Vector* pPt, void* pUser)
{
	struct zclip_data* pdata = pUser;

	return (pPt->z >= pdata->zlim) ? TRUE : FALSE;
}

boolean 
zmax_clip(NB_Vector* pPt, void* pUser)
{
	struct zclip_data* pdata = pUser;

	return (pPt->z <= pdata->zlim) ? TRUE : FALSE;
}

void 
z_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* pUser)
{
	struct zclip_data* pdata = pUser;
	double x1 = pPt1->x;
	double y1 = pPt1->y;
	double z1 = pPt1->z;
	double x2 = pPt2->x;
	double y2 = pPt2->y;
	double z2 = pPt2->z;
	double zclip = pdata->zlim;

	pPtOut->x = x2 + ((zclip-z2)/(z1-z2))*(x1-x2); 
	pPtOut->y = y2 + ((zclip-z2)/(z1-z2))*(y1-y2); 
	pPtOut->z = zclip;
}

boolean
xzmax_clip(NB_Vector* pPt, void* pUser)
{
	struct tanclip_data* pdata = pUser;

	return (pPt->x < pdata->tan * pPt->z) ? TRUE : FALSE;
}

boolean
xzmin_clip(NB_Vector* pPt, void* pUser)
{
	struct tanclip_data* pdata = pUser;

	return (pPt->x > pdata->tan * pPt->z) ? TRUE : FALSE;
}

void 
xz_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* pUser)
{
	struct tanclip_data* pdata = pUser;
	double x1 = pPt1->x;
	double y1 = pPt1->y;
	double z1 = pPt1->z;
	double x2 = pPt2->x;
	double y2 = pPt2->y;
	double z2 = pPt2->z;
	double dx, dy, dz, t;

	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;

	if (dz != 0 || dx != 0)
	{
		t = (pdata->tan*z1 - x1)/(dx + pdata->tan*dz);
		if (t > 1) // if t is not 0..1, then don't clip the line
			*pPtOut = *pPt2;
		else if (t < 0)
			*pPtOut = *pPt1;
		else
		{
			pPtOut->x = x1 + dx*t;
			pPtOut->y = y1 + dy*t;
			pPtOut->z = z1 + dz*t;
		}
	}
	else // line is on the plane.  Return the first point.
		*pPtOut = *pPt1;
	
}

boolean
yzmax_clip(NB_Vector* pPt, void* pUser)
{
	struct tanclip_data* pdata = pUser;

	return (pPt->y < pdata->tan * pPt->z) ? TRUE : FALSE;
}

boolean
yzmin_clip(NB_Vector* pPt, void* pUser)
{
	struct tanclip_data* pdata = pUser;

	return (pPt->y > pdata->tan * pPt->z) ? TRUE : FALSE;
}

void 
yz_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* pUser)
{
	struct tanclip_data* pdata = pUser;
	double x1 = pPt1->x;
	double y1 = pPt1->y;
	double z1 = pPt1->z;
	double x2 = pPt2->x;
	double y2 = pPt2->y;
	double z2 = pPt2->z;
	double dx, dy, dz, t;

	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;

	if (dz != 0 || dy != 0)
	{
		t = (pdata->tan*z1 - y1)/(dy + pdata->tan*dz);
		if (t > 1) // if t is not 0..1, then don't clip the line
			*pPtOut = *pPt2;
		else if (t < 0)
			*pPtOut = *pPt1;
		else
		{
			pPtOut->x = x1 + dx*t;
			pPtOut->y = y1 + dy*t;
			pPtOut->z = z1 + dz*t;
		}
	}
	else // line is on the plane.  Return the first point.
		*pPtOut = *pPt1;

}

boolean
ortho_clip(NB_Vector* pPt, void* pUser)
{
	struct ortho_clip_data* pdata = pUser;

	if (pdata->clipflags & ORTHO_CLIP_X)
	{
		return (boolean)((pdata->clipflags & ORTHO_CLIP_MIN) ? pPt->x >= pdata->val : pPt->x <= pdata->val);
	}
	else if (pdata->clipflags & ORTHO_CLIP_Y)
	{
		return (boolean)((pdata->clipflags & ORTHO_CLIP_MIN) ? pPt->y >= pdata->val : pPt->y <= pdata->val);
	}
	else if (pdata->clipflags & ORTHO_CLIP_Z)
	{
		return (boolean)((pdata->clipflags & ORTHO_CLIP_MIN) ? pPt->z >= pdata->val : pPt->z <= pdata->val);
	}

	return FALSE;
}

void 
ortho_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* pUser)
{
	struct ortho_clip_data* pdata = pUser;
	double x1 = pPt1->x;
	double y1 = pPt1->y;
	double z1 = pPt1->z;
	double x2 = pPt2->x;
	double y2 = pPt2->y;
	double z2 = pPt2->z;
	double dx, dy, dz, t = 0;

	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;

	if (pdata->clipflags & ORTHO_CLIP_X)
	{
		if (dx == 0)
			t = 0;
		else
			t = (pdata->val - x1) / dx;
	}
	else if (pdata->clipflags & ORTHO_CLIP_Y)
	{
		if (dy == 0)
			t = 0;
		else
			t = (pdata->val - y1) / dy;
	}
	else if (pdata->clipflags & ORTHO_CLIP_Z)
	{
		if (dz == 0)
			t = 0;
		else
			t = (pdata->val - z1) / dz;
	}

	pPtOut->x = x1 + dx * t;
	pPtOut->y = y1 + dy * t;
	pPtOut->z = z1 + dz * t;
}

boolean
vector_intercept( NB_Vector* vout, NB_Vector* p1, NB_Vector* p2, NB_Vector* q1, NB_Vector* q2 ){
	// Find intersection of two lines
	//	Line1 defined by points p1, p2 (assumes z is zero)
	//	Line2 defined by points q1, q2 (assumes z is zero)
	//	Returns TRUE iff intersection found, FALSE if they are parallel or close to parallel
	double			a1, b1, c1, a2, b2, c2, det;
	a1 = p2->y - p1->y;
	b1 = p1->x - p2->x;
	c1 = a1*p1->x + b1*p1->y;
	a2 = q2->y - q1->y;
	b2 = q1->x - q2->x;
	c2 = a2*q1->x + b2*q1->y;
	det = a1*b2 - a2*b1;
	if( det == 0. ) return FALSE;
	vout->x = (b2*c1 - b1*c2)/det;
	vout->y = (a1*c2 - a2*c1)/det;
	vout->z = 0.;
	return TRUE;
}

boolean
vector_points_within( NB_Vector* p1, NB_Vector* p2, double distance ){
	// Returns TRUE iff the distance from p1 to p2 is <= distance
	NB_Vector			delta;
	vector_subtract( &delta, p1, p2 );
	return (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z < distance*distance) ? TRUE : FALSE;
}

boolean
vector_point_in_segment( NB_Vector* pt, NB_Vector* end1, NB_Vector* end2, double epsilon )
{
	// Return TRUE iff pt is in the line segment [end1,end2] outset by epsilon in each dimension
	double xmin, ymin, xmax, ymax;
	if( end1->x <= end2->x )
    { 
        xmin = end1->x; xmax = end2->x; 
    }
	else										
    { 
        xmin = end2->x; xmax = end1->x; 
    }
	if( end1->y <= end2->y )
    { 
        ymin = end1->y; ymax = end2->y; 
    }
	else										
    { 
        ymin = end2->y; ymax = end1->y; 
    }
	if( xmin-epsilon <= pt->x && pt->x <= xmax+epsilon && ymin-epsilon <= pt->y && pt->y <= ymax+epsilon )
    {
		return TRUE;
    }
	return FALSE;
}


// Local Functions ...................................................................................................

/*! Project all points of a polygon to a line and return the min/max of the projection points.

    @see vector_polygons_intersect
*/
void
ProjectPolygonToLine(const NB_Vector* line,     /*!< Line to which to project */
                     const NB_Vector* polygon,  /*!< Polygon to project */
                     int count,                 /*!< Number of points in polygon */
                     double* min,               /*!< On return the minimum point of the projection */
                     double* max                /*!< On return the maximum point of the projection */
                     )
{
    int j = 0;
    double dotProduct = 0;
    *min = 0;
    *max = 0;


    if (count == 0)
    {
        return;
    }

    // Project point to line
    dotProduct = vector_dot_product(line, &polygon[0]);
    
    *min = dotProduct;
    *max = dotProduct;
    
    // For all points in polygon
    for (j = 1; j < count; ++j)
    {
        // Project point to line
        double dotProduct = vector_dot_product(line, &polygon[j]);

        *min = MIN(*min, dotProduct);
        *max = MAX(*max, dotProduct);
    }
}

/*! Get the edge vector of a polygon (path around a polygon) 

    @see vector_polygons_intersect
    @see vector_is_point_in_polygon
*/
void
GetPolygonEdge(const NB_Vector* polygon,    /*!< Polygon to check */
               int count,                   /*!< Number of points in polygon */
               int i,                       /*!< Start index of path/edge */
               double* x1,                  /*!< On return start coordinate of path/edge */
               double* y1,                  /*!< On return start coordinate of path/edge */
               double* x2,                  /*!< On return end coordinate of path/edge */
               double* y2                   /*!< On return end coordinate of path/edge */
               )
{
    // Start of path
    *x1 = polygon[i].x;
    *y1 = polygon[i].y;

    // End of path
    if ((i + 1) < count)
    {
        *x2 = polygon[i + 1].x;
        *y2 = polygon[i + 1].y;
    }
    else
    {
        // Close polygon, use start point
        *x2 = polygon[0].x;
        *y2 = polygon[0].y;
    }
}

/*! This function projects the first polygon to the second polygon.

    This does half the work for vector_polygons_intersect(). See vector_polygons_intersect() for more details.

    @see vector_polygons_intersect
*/
boolean
ProjectPolygon(const NB_Vector* polygon1,   /* See vector_polygons_intersect */
               int count1,
               boolean isRectangle1,
               const NB_Vector* polygon2,
               int count2,
               boolean isRectangle2)
{
    int i = 0;

    // If we have a rectangle then we only need to check half the edges since the 3rd and 4th edge is parallel
    int count = isRectangle1 ? (count1 / 2) : count1;

    // For all edges from polygon one
    for (i = 0; i < count; ++i)
    {
        double x1 = 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = 0;

        double min1 = 0;
        double min2 = 0;
        double max1 = 0;
        double max2 = 0;

        // Perpendicular to edge
        NB_Vector normal = {0};

        // Get start/end of path
        GetPolygonEdge(polygon1, count1, i, &x1, &y1, &x2, &y2);

        // We don't actually need to normalize the vector (set length to one)
        normal.x =  y2 - y1;
        normal.y = -(x2 - x1);

        /*
            Now project all the points from both polygons to the normal. Do this using the dot-product.
            We only care for the min/max for each polygon.
        */
        ProjectPolygonToLine(&normal, polygon1, count1, &min1, &max1);
        ProjectPolygonToLine(&normal, polygon2, count2, &min2, &max2);

        // If there is a gap then the polygons don't intersect
        if ((min1 > max2) || (min2 > max1))
        {
            return FALSE;
        }
    }

    // All projections overlap
    return TRUE;
}


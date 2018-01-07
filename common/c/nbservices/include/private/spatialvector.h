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
 * vector.h: created 2003/03/17 by Mark Goddard.
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
#ifndef SPATIAL_VECTOR_H
#define SPATIAL_VECTOR_H

#include "nbspatial.h"
#include "cslutil.h"

#define ORTHO_CLIP_X		0x1
#define ORTHO_CLIP_Y		0x2
#define ORTHO_CLIP_Z		0x4
#define ORTHO_CLIP_MIN		0x8
#define ORTHO_CLIP_MAX		0x10

struct tristrip 
{
	NB_Vector* points;
	int npoints;
	boolean fg;
};

struct tanclip_data 
{
	double tan;
};

struct ortho_clip_data 
{
	int clipflags;
	double val;
};

struct zclip_data 
{
	double zlim;
};


void        vector_set(NB_Vector* output, double x, double y, double z);
double		vector_magnitude(const NB_Vector* px);

void		vector_cross_product(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py);
double		vector_dot_product(const NB_Vector* px, const NB_Vector* py);

void		vector_from_lat_lon(NB_Vector* pout, double lat, double lon);
void		vector_to_lat_lon(double* plat, double* plon, const NB_Vector* pv);

void		vector_normalize(NB_Vector* pout, const NB_Vector* pv);
void		vector_normalize_to_length(NB_Vector* pout, const NB_Vector* pv, double length_out);
void		vector_multiply(NB_Vector* pout, const NB_Vector* pv, double v);
void		vector_subtract(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py);
void		vector_add(NB_Vector* pout, const NB_Vector* px, const NB_Vector* py);
boolean		vector_intercept( NB_Vector* vout, NB_Vector* p1, NB_Vector* p2, NB_Vector* q1, NB_Vector* q2 );
boolean		vector_points_within( NB_Vector* p1, NB_Vector* p2, double distance );
boolean		vector_point_in_segment( NB_Vector* pt, NB_Vector* end1, NB_Vector* end2, double epsilon );

double	    vector_distance(const NB_Vector* px, const NB_Vector* py);
boolean	    vector_is_point_on_line(const NB_Vector* pp, const NB_Vector* pa, const NB_Vector* pb);
int			project_point_to_line(double p_lat, double p_lon, double l1_lat, double l1_lon1, double l2_lat, double l2_lon, double* p_proj_lat, double* p_proj_lon, double* pdist);

/*! Determines if a point is inside a convex polygon.

    This function assumes that the polygon is convex! This allows for a simpler algorithm.

    This function checks the x/y-plane and ignores the z-values.

    @return 'TRUE' if the point is inside, 'FALSE' otherwise
*/
boolean     
vector_is_point_in_polygon(const NB_Vector* point,      /*!< Point to check */
                           const NB_Vector* polygon,    /*!< Points of convex polygon, in clockwise order */
                           int pointsInPolygon          /*!< Number of points in polygon */
                           );

/*! Check if two given convex polygons intersect.
    
    This function assumes that the polygons are convex! This allows for a simpler algorithm.

    This function checks the x/y-plane and ignores the z-values.

    @return 'TRUE' if the polygons intersect, 'FALSE' otherwise.
*/
boolean
vector_polygons_intersect(const NB_Vector* polygon1,    /*!< Points of first convex polygon */
                          int count1,                   /*!< Number of points in first polygon */
                          boolean isRectangle1,         /*!< Set to 'TRUE' if the first polygon is a rectangle */
                          const NB_Vector* polygon2,    /*!< Points of second convex polygon */
                          int count2,                   /*!< Number of points in second polygon */
                          boolean isRectangle2          /*!< Set to 'TRUE' if the second polygon is a rectangle */
                          );

void        calculate_segment_normals(NB_Vector* p1, NB_Vector* p2, double normal_length,
                NB_Vector* n1l, NB_Vector* n1r, NB_Vector* n2l, NB_Vector* n2r, 
                double* plengthremain, NB_Vector* last_pt, NB_Vector* last_dir,
                double alt_normal_length, NB_Vector* alt_n1l, NB_Vector* alt_n1r, NB_Vector* alt_n2l, NB_Vector* alt_n2r);

typedef boolean (*clippointfunc)(NB_Vector* pPt, void* userData);
typedef void (*intersectfunc)(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* userData);

int         sutherland_hodgman_polygon_clip(NB_Vector* pin, int in_len, NB_Vector* pout, int out_size,
									clippointfunc clipper, intersectfunc intersect, void* userdata);

boolean		zmin_clip(NB_Vector* pPt, void* userData);
boolean		zmax_clip(NB_Vector* pPt, void* userData);
void		z_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* userData);

boolean		ortho_clip(NB_Vector* pPt, void* userData);
void		ortho_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* userData);

boolean		xzmin_clip(NB_Vector* pPt, void* userData);
boolean		xzmax_clip(NB_Vector* pPt, void* userData);
void		xz_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* userData);

boolean		yzmin_clip(NB_Vector* pPt, void* userData);
boolean		yzmax_clip(NB_Vector* pPt, void* userData);
void		yz_intersect(NB_Vector* pPt1, NB_Vector* pPt2, NB_Vector* pPtOut, void* userData);

#endif


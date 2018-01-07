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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_fuel_pricesummary.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_fuel_pricesummary.h"
#include "vec.h"

NB_Error
data_fuel_pricesummary_init(data_util_state* pds, data_fuel_pricesummary* fps)
{
	NB_Error err = NE_OK;

	fps->vec_fuelproducts	=	CSL_VectorAlloc(sizeof(data_fuelproduct));

	return err;
}

void		
data_fuel_pricesummary_free(data_util_state* pds, data_fuel_pricesummary* fps)
{
	int n,l;

	if (fps->vec_fuelproducts) {
		l = CSL_VectorGetLength(fps->vec_fuelproducts);

		for (n=0;n<l;n++)
			data_fuelproduct_free(pds, (data_fuelproduct*) CSL_VectorGetPointer(fps->vec_fuelproducts, n));

		CSL_VectorDealloc(fps->vec_fuelproducts);
	}

	fps->vec_fuelproducts = NULL;
}

NB_Error	
data_fuel_pricesummary_from_tps(data_util_state* pds, data_fuel_pricesummary* fps, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_fuelproduct fuel_product;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_fuel_pricesummary_free(pds, fps);

	err = err ? err : data_fuel_pricesummary_init(pds, fps);

	err = err ? err : data_fuelproduct_init(pds, &fuel_product);

	if (err != NE_OK)
		return err;

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "fuel-product") == 0) {

			err = data_fuelproduct_from_tps(pds, &fuel_product, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(fps->vec_fuelproducts, &fuel_product))
					err = NE_NOMEM;

				if (err == NE_OK)
					data_fuelproduct_init(pds, &fuel_product); // clear out the data since we have copied it (shallow copy)
				else
					data_fuelproduct_free(pds, &fuel_product); // free the data if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}
	
errexit:
	if (err != NE_OK)
		data_fuel_pricesummary_free(pds, fps);
	return err;
}

boolean		
data_fuel_pricesummary_equal(data_util_state* pds, data_fuel_pricesummary* fps1, data_fuel_pricesummary* fps2)
{
	int ret;
	int n,l;

	ret = ((l = CSL_VectorGetLength(fps1->vec_fuelproducts)) == CSL_VectorGetLength(fps2->vec_fuelproducts));
	
	for (n = 0; n < l && ret; n++)
		ret = ret && data_fuelproduct_equal(pds, (data_fuelproduct*) CSL_VectorGetPointer(fps1->vec_fuelproducts, n), (data_fuelproduct*) CSL_VectorGetPointer(fps2->vec_fuelproducts, n));

	return (boolean) ret;
}

NB_Error	
data_fuel_pricesummary_copy(data_util_state* pds, data_fuel_pricesummary* fps_dest, data_fuel_pricesummary* fps_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_fuelproduct	fuel_product;

	data_fuelproduct_init(pds, &fuel_product);

	data_fuel_pricesummary_free(pds, fps_dest);
	err = err ? err : data_fuel_pricesummary_init(pds, fps_dest);
	
	l = CSL_VectorGetLength(fps_src->vec_fuelproducts);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_fuelproduct_copy(pds, &fuel_product, CSL_VectorGetPointer(fps_src->vec_fuelproducts, n));
		err = err ? err : CSL_VectorAppend(fps_dest->vec_fuelproducts, &fuel_product) ? NE_OK : NE_NOMEM;

		if (err)
			data_fuelproduct_free(pds, &fuel_product);
		else
			data_fuelproduct_init(pds, &fuel_product);
	}


	return err;
}

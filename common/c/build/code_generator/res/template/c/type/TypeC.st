TypeC(type) ::= <<
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
 * data_<type.name;format="camelCaseToUnderscore">.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_<type.name;format="camelCaseToUnderscore">.h"

NB_Error	
data_<type.name;format="camelCaseToUnderscore">_init(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>)
{
	NB_Error err = NE_OK;

	err = err ? err : data_point_init(dataState, &p<type.name>->point);
	err = err ? err : data_gps_init(dataState, &p<type.name>->gps);
	err = err ? err : data_string_init(dataState, &p<type.name>->variant);

    position->boxValid = FALSE;
	err = err ? err : data_box_init(dataState, &p<type.name>->boundingBox);

	return err;
}

void	
data_<type.name;format="camelCaseToUnderscore">_free(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>)
{
	data_point_free(dataState, &p<type.name>->point);
	data_gps_free(dataState, &p<type.name>->gps);
	data_string_free(dataState, &p<type.name>->variant);
	data_box_free(dataState, &p<type.name>->boundingBox);
}

tpselt		
data_<type.name;format="camelCaseToUnderscore">_to_tps(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>, const char *elementName)
{
	tpselt newElement = NULL;
<type.fields: {field |
    tpselt <field.wireName;format="dashesToUnderscore">Element = NULL;
}>

	newElement = te_new(elementName);
    if (!te)
    {
        goto errexit;
    }

<type.fields: {field |
    if ((<field.wireName;format="dashesToUnderscore">Element = data_<field.type.name;format="camelCaseToUnderscore">_to_tps(dataState, &p<type.name>-><field.type.name;format="camelCaseToUnderscore">)) != NULL && te_attach(newElement, <field.wireName;format="dashesToUnderscore">Element))
    {
	    <field.wireName;format="dashesToUnderscore">Element = NULL;
    \}
    else
    {
	    goto errexit;
    \}
}>


	return newElement;

errexit:

	if (newElement) 
    {
        te_dealloc(newElement);
    }

	return NULL;
}

NB_Error	
data_<type.name;format="camelCaseToUnderscore">_from_tps(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>, tpselt element)
{
	NB_Error err = NE_OK;
	tpselt	childElement = NULL;
	
	if (element == NULL) 
    {
		err = NE_INVAL;
		goto errexit;
	}

<type.fields: {field |
	err = data_<field.type.name;format="camelCaseToUnderscore">_init(dataState, p<type.name>);
	if (err != NE_OK)
    {
		return err;
    \}
}>

<type.fields: {field |
	childElement = te_getchild(element, "<field.wireName>");

	if (childElement == NULL) 
    {
		err = NE_INVAL;
		goto errexit;
	\}

	err = err ? err : data_<field.type.name;format="camelCaseToUnderscore">_from_tps(dataState, &p<type.name>-><field.type.name;format="camelCaseToUnderscore">, childElement);
}>

errexit:
	if (err != NE_OK)
    {
		data_position_free(dataState, position);
    }
	return err;
}

boolean		
data_<type.name;format="camelCaseToUnderscore">_equal(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>1, data_<type.name;format="camelCaseToUnderscore">* p<type.name>2)
{
	return (data_string_equal(dataState, &p<type.name>1->variant, &p<type.name>2->variant) &&
			data_point_equal(dataState, &p<type.name>1->point, &p<type.name>2->point) &&
			data_gps_equal(dataState, &p<type.name>1->gps, &p<type.name>2->gps) &&
            (p<type.name>1->boxValid == p<type.name>2->boxValid) &&
            data_box_equal(dataState, &p<type.name>1->boundingBox, &p<type.name>2->boundingBox));
}

NB_Error	
data_<type.name;format="camelCaseToUnderscore">_copy(data_util_state* dataState, data_<type.name;format="camelCaseToUnderscore">* p<type.name>_dest, data_<type.name;format="camelCaseToUnderscore">* p<type.name>_src)
{
	NB_Error err = NE_OK;

	data_<type.name;format="camelCaseToUnderscore">_free(dataState, p<type.name>_dest);
	data_<type.name;format="camelCaseToUnderscore">_init(dataState, p<type.name>_dest);

	err = err ? err : data_string_copy(dataState, &p<type.name>_dest->variant, &p<type.name>_src->variant);
	err = err ? err : data_point_copy(dataState, &p<type.name>_dest->point, &p<type.name>_src->point);
	err = err ? err : data_gps_copy(dataState, &p<type.name>_dest->gps, &p<type.name>_src->gps);

    p<type.name>_dest->boxValid = p<type.name>_src->boxValid;

	err = err ? err : data_box_copy(dataState, &p<type.name>_dest->boundingBox, &p<type.name>_src->boundingBox);

	return err;
}

>>
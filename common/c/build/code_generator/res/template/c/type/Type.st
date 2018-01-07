Type(type) ::= <<
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
 * data_<type.name;format="camelCaseToUnderscore">.h: created 2004/12/21 by Mark Goddard.
 */

#ifndef <Define(type, "")>
#define <Define(type, "")>

#include "datautil.h"
#include "data_position.h"
#include "nbexp.h"

typedef struct data_<type.name;format="camelCaseToUnderscore">_ {
	<type.fields: { field |
	data_<field.type.name;format="camelCaseToUnderscore"> <field.type.name;format="camelCaseToUnderscore">;  
	}>
	<if(type.isAbstract)>
	data_string <type.discriminatorName> = <subType.discriminatorValue>; 
	<field.subTypes: { subType | 
	data_<subType.name;format="camelCaseToUnderscore"> <subType.name;format="camelCaseToUnderscore">;
	}>
	<endif>
} data_<type.name;format="camelCaseToUnderscore">;

NB_Error	data_<type.name;format="camelCaseToUnderscore">_init(data_util_state* pds, data_<type.name;format="camelCaseToUnderscore">* p<type.name;format="toInitials">);
void		data_<type.name;format="camelCaseToUnderscore">_free(data_util_state* pds, data_<type.name;format="camelCaseToUnderscore">* p<type.name;format="toInitials">);

tpselt		data_<type.name;format="camelCaseToUnderscore">_to_tps(data_util_state* pds, data_<type.name;format="camelCaseToUnderscore">* p<type.name;format="toInitials">);

boolean		data_<type.name;format="camelCaseToUnderscore">_equal(data_util_state* pds, data_<type.name;format="camelCaseToUnderscore">_query* p<type.name;format="toInitials">1, data_<type.name;format="camelCaseToUnderscore">* p<type.name;format="toInitials">2);
NB_Error	data_<type.name;format="camelCaseToUnderscore">_copy(data_util_state* pds, data_<type.name;format="camelCaseToUnderscore">_query* p<type.name;format="toInitials">_dest, data_<type.name;format="camelCaseToUnderscore">* p<type.name;format="toInitials">_src);

#endif // <Define(type, "")>

>>
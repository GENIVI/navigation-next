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

/*!--------------------------------------------------------------------------

    @file     data_url_args_template.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_URL_ARGS_TEMPLATE_H
#define DATA_URL_ARGS_TEMPLATE_H

#include "datautil.h"
#include "data_string.h"
#include "data_url_args_template_characteristics.h"
typedef struct data_url_args_template_
{
    /* Child Elements */
    CSL_Vector*     vec_metadata;           /*!< These elements will be primarily added for city
                                                 models to provide additional data details. */

    CSL_Vector*     vec_formats;            /*!< Data formats in which the content can be
                                                 retrieved (e.g. PNG or NBM).  The client must
                                                 specify one of these formats when requesting the
                                                 data.*/

    data_url_args_template_characteristics characteristics;
                                            /*!< Specifies the characteristics of the type of data
                                                 returned through this URL.*/
    /* Attributes */
    data_string     type;                   /*!< Type of the data to be downloaded "speed signs",
                                                 "junctions", "realistic signs", "city models",
                                                 "textures", and "map tiles" */
    data_string     template;               /*!< URL template */
    data_string     tile_store_template;    /*!< The template for the tile cache. */
    uint32          tile_size;              /*!< The tile size to use for this server, based on
                                                 the screen size and resolution provided in the
                                                 maptile-source request.
                                                 Ignored if type is not map tile. */
    uint32          min_zoom;               /*!< Minimum zoom level supported for this source
                                                 Ignored if type is not map tile. */
    uint32          max_zoom;               /*!< Maximum zoom level supported for this source
                                                 Ignored if type is not map tile. */
    uint32          cache_id;               /*!< Number that is used by the client to see if it
                                                 needs to flush the cache. */
    uint32          ref_tile_grid_level;    /*!< Reference Tile Grid Level*/
    uint32          lam_index;              /*!< Index (bit offset) within the LAM (layer
                                                 availability matrix) bitmask for this layer.*/
    uint32          base_draw_order;        /*!< Default order for drawing the features in this
                                                 layer within the map. */
    uint32          label_draw_order;       /*!< The order to draw text labels in this layer,
                                                 relative to other text labels and features in the
                                                 map (i.e. the drawing of features and their
                                                 associated labels may be interleaved). Valid range
                                                 is 1 (0 is assigned to the background layer) to 255
                                                 (for the topmost feature or label).*/
    uint32          download_priority;      /*!< Priority to download from the content source server
                                             relative to the other layers.  Valid range is 0
                                             (for download first) to 255 (for download last).*/

    data_string     ds_gen_id;              /*!< Dataset Generation ID.  This ID uniquely
                                                 specifies the generation (build) of the map
                                                 content for the layer.*/

} data_url_args_template;

NB_Error    data_url_args_template_init(data_util_state* state, data_url_args_template* urlArgsTemplate);
void        data_url_args_template_free(data_util_state* state, data_url_args_template* urlArgsTemplate);

NB_Error    data_url_args_template_from_tps(data_util_state* state, data_url_args_template* urlArgsTemplate, tpselt tpsElement);

boolean     data_url_args_template_equal(data_util_state* state, data_url_args_template* urlArgsTemplate1, data_url_args_template* urlArgsTemplate2);
NB_Error    data_url_args_template_copy(data_util_state* state, data_url_args_template* destinationUrlArgsTemplate, data_url_args_template* sourceUrlArgsTemplate);

#endif // DATA_URL_ARGS_TEMPLATE_H

/*! @} */

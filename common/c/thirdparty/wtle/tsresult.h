/*****************************************************************************
 *
 *  tsresult.h - Defines result and error codes.
 *
 *  Copyright (C) 2003-2006 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSRESULT_H
#define TSRESULT_H

TS_BEGIN_HEADER

/* No error */
#define TS_OK                                         (TsResult)0

/* General errors */
#define TS_ERR                                        (TsResult)1
#define TS_ERR_ARGUMENT_IS_NULL_PTR                   (TsResult)2
#define TS_ERR_ARGUMENT_OUT_OF_RANGE                  (TsResult)3
#define TS_ERR_DATA_SIZE_INVALID                      (TsResult)4
#define TS_ERR_INVALID_MODE                           (TsResult)5
#define TS_ERR_INVALID_TAG_ARGUMENT                   (TsResult)6
#define TS_ERR_RETURNED_STRING_WAS_TRUNCATED          (TsResult)7
#define TS_ERR_DATA_HAS_NULL_PTR                      (TsResult)8
#define TS_ERR_INVALID_RESOURCE                       (TsResult)9
#define TS_ERR_FUNCTION_NOT_SUPPORTED                 (TsResult)10

/* Layout module errors 100+ */
#define TS_ERR_POINT_NOT_IN_LAYOUT                  (TsResult)101
#define TS_ERR_FONT_STYLE_NOT_SET                   (TsResult)102
#define TS_ERR_LINE_EXCEEDED_ASCENDER_LIMIT         (TsResult)103
#define TS_ERR_LINE_EXCEEDED_DESCENDER_LIMIT        (TsResult)104
#define TS_ERR_INVALID_INLINE_IMAGE_INDEX           (TsResult)105
#define TS_ERR_COULD_NOT_FIT_WIDTH                  (TsResult)106
#define TS_ERR_NO_CHARACTERS_LEFT_TO_COMPOSE        (TsResult)107
#define TS_ERR_INVALID_BYTE_ORDER_MARK              (TsResult)108
#define TS_ERR_INVALID_RESOURCE_VERSION             (TsResult)109
#define TS_ERR_INVALID_RESOURCE_TAG                 (TsResult)110
#define TS_ERR_INVALID_CONTAINER_ID                 (TsResult)111
#define TS_ERR_NO_CONTAINERS_SPECIFIED              (TsResult)112
#define TS_ERR_INVALID_LAYOUT_MODE                  (TsResult)113
#define TS_ERR_INVALID_SHAPE_ID                     (TsResult)114
#define TS_ERR_UNSUPPORTED_GLYPHS_FLOW              (TsResult)115
#define TS_ERR_UNSUPPORTED_LINE_FLOW                (TsResult)116
#define TS_ERR_INVALID_CARET                        (TsResult)117
#define TS_ERR_LINE_SPACING_ATTRIBUTES_CHANGED      (TsResult)118
#define TS_ERR_INVALID_INLINE_IMAGE_HANDLE          (TsResult)119
#define TS_ERR_INLINE_IMAGE                         (TsResult)120
#define TS_ERR_INLINE_IMAGE_CALLBACKS_NOT_SET       (TsResult)121

/* Base module errors 200+*/

/* Memory errors */
#define TS_ERR_MALLOC_FAIL                          (TsResult)201
#define TS_ERR_REALLOC_FAIL                         (TsResult)202
#define TS_ERR_FREE_FAIL                            (TsResult)203
#define TS_ERR_FREE_BAD_PTR                         (TsResult)204
#define TS_ERR_REALLOC_BAD_PTR                      (TsResult)205
#define TS_ERR_RESIZE_FAIL                          (TsResult)206
#define TS_ERR_BAD_CHECKSUM                         (TsResult)207
#ifdef TS_INT_MEM
#define TS_ERR_BLOCK_SIZE                           (TsResult)208
#define TS_ERR_BLOCK_BEFORE_FIRST                   (TsResult)209
#define TS_ERR_BLOCK_AFTER_LAST                     (TsResult)210
#define TS_ERR_BLOCK_DAMAGED                        (TsResult)211
#define TS_TS_ERR_BLOCK_IS_FREE                     (TsResult)212
#define TS_ERR_BLOCK_IN_USE                         (TsResult)213
#define TS_ERR_FREE_LIST_DAMAGED                    (TsResult)214
#define TS_ERR_BLOCK_GRANULARITY                    (TsResult)215
#endif

/* Mutex errors */
#define TS_ERR_MUTEX_WAIT_TIMEOUT                   (TsResult)221
#define TS_ERR_MUTEX_WAIT_ABANDONED                 (TsResult)222
#define TS_ERR_INVALID_RESOURCE_TYPE                (TsResult)223

/* File I/O errors */
#define TS_ERR_ON_FILE_OPEN                         (TsResult)230
#define TS_ERR_ON_FILE_READ                         (TsResult)231
#define TS_ERR_ON_FILE_SEEK                         (TsResult)232
#define TS_ERR_ON_FILE_REWIND                       (TsResult)233
#define TS_ERR_ON_FILE_CLOSE                        (TsResult)234

/* Device Context errors and rendering errors */
#define TS_ERR_DC                                   (TsResult)300 /* general device context error */
#define TS_ERR_IMAGE_TYPE_NOT_SUPPORTED             (TsResult)301
#define TS_ERR_GLYPH_IMAGE_DATA                     (TsResult)302


/* Font Services module errors 400+ */
#define TS_ERR_FM_INIT_FONT_ENGINE_FAILED           (TsResult)401
#define TS_ERR_FM_DONE_FONT_ENGINE_FAILED           (TsResult)402
#define TS_ERR_FM_INIT_FONT_FAILED                  (TsResult)403
#define TS_ERR_FM_DONE_FONT_FAILED                  (TsResult)404
#define TS_ERR_FM_INIT_STYLE_FAILED                 (TsResult)405
#define TS_ERR_FM_DONE_STYLE_FAILED                 (TsResult)406
#define TS_ERR_FM_GET_TABLE_FAILED                  (TsResult)407
#define TS_ERR_FM_RELEASE_TABLE_FAILED              (TsResult)408
#define TS_ERR_FM_MAPCHAR_FAILED                    (TsResult)409
#define TS_ERR_FM_INIT_GLYPH_FAILED                 (TsResult)410
#define TS_ERR_FM_DONE_GLYPH_FAILED                 (TsResult)411
#define TS_ERR_FM_GET_GLYPH_PTS_FAILED              (TsResult)412
#define TS_ERR_FM_INVALID_CHARID                    (TsResult)413
#define TS_ERR_FM_INVALID_FONTID                    (TsResult)414
#define TS_ERR_FM_CMAP_PROBLEM                      (TsResult)415
#define TS_ERR_FM_UNSUPPORTED_GLYPHDATAFORMAT       (TsResult)416
#define TS_ERR_FM_GET_GLYPH_SCALE_FAILED            (TsResult)417

/** Shaper module errors 500+ */
#define TS_ERR_NO_KERNING                           (TsResult)500

#define TS_CHECK_RESULT(result)                     {if (result != TS_OK) return result;}

#define TS_RETURN_NULL_IF_NULL(p)                   {if (p == NULL) return(NULL);}
#define TS_RETURN_ERROR_IF_NULL(p, err)             {if (p == NULL) return(err);}
#define TS_RETURN_VALUE_IF_NULL(p, val)             {if (p == NULL) return(val);}
#define TS_RETURN_VOID_IF_NULL(p)                   {if (p == NULL) return;}

#define TS_ASSERT_AND_RETURN_ERROR_IF_NULL(p, err)  {TS_ASSERT(p != NULL); TS_RETURN_ERROR_IF_NULL(p, err);}
#define TS_ASSERT_AND_RETURN_VALUE_IF_NULL(p, val)  {TS_ASSERT(p != NULL); TS_RETURN_VALUE_IF_NULL(p, val);}
#define TS_ASSERT_AND_RETURN_NULL_IF_NULL(p)        {TS_ASSERT(p != NULL); TS_RETURN_NULL_IF_NULL(p);}
#define TS_ASSERT_AND_RETURN_VOID_IF_NULL(p)        {TS_ASSERT(p != NULL); TS_RETURN_VOID_IF_NULL(p);}

#define TS_ASSERT_AND_RETURN_ERROR_IF_NULL_ARG(p)   {TS_ASSERT_AND_RETURN_ERROR_IF_NULL(p, TS_ERR_ARGUMENT_IS_NULL_PTR);}


#define TS_IS_ERROR(result)                         ((result) != TS_OK)


TS_END_HEADER

#endif /* TSRESULT_H */

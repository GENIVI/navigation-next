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

@file     Sprite.h
@date     03/05/2009
@defgroup MOBIUS_MAP  Mobius Map Draw API
@author   John Efseaff



*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#pragma once
#ifndef SPRITE_H_ONCE
#define SPRITE_H_ONCE


#include "NcdbTypes.h"
#include "AutoSharedPtr.h"
#include "AutoArray.h"
#include "FileName.h"
#include <stdio.h>

namespace Ncdb {

#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<unsigned char>;
#endif

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
} PAL_ENTRY_COLOR;
typedef struct image_source
{
	unsigned char* data;
	int size;
	int offset;
	image_source(){data = 0;size = 0;offset=0;}
    int getBufferSize(){return size;}
}IMGAGESOURCE;

#ifdef _MSC_VER
template class NCDB_API AutoArray<PAL_ENTRY_COLOR>;
#endif

//! @brief 2D Graphic Bitmap Class
//! @details The sprite class defines 2D graphic objects that shall not get
//! texture mapped into a 3D environment.  They shall remain 2D flat
//! bitmap objects.

class NCDB_API Sprite
{
public:
    //! Constructor
	Sprite(void);

	//! Copy constructor.
    Sprite(const Sprite& other);

	//! Copy operator.
	void operator = (const Sprite& other);

	//! Comparison operator.
	bool operator == (class Sprite const &sprite) const;

	//! Comparison operator.
	bool operator == (class FileName const &file) const;

	//! Destructor
	virtual ~Sprite(void);

	//! Return the image buffer pointer.
	unsigned char* getImageBuffer(void) const {return (unsigned char *)m_ImageBuffer;}

	//! Return the image buffer size.
	int getBufferSize(void) const {return m_BufferSize;}

	//! @brief Get the bitmap width in X pixels.
	int getImageWidth(void) const {return m_Width;}

	//! @brief Get the bitmap height in Y pixels.
	int getImageHeight(void) const {return m_Height;}

	//! @brief Get the image color depth.
	ColorMode getColorMode(void) const {return m_ColorMode;}

	//! @brief Get the number of bytes per row.
	int getStride(void) const;

	//! @brief Get the number of bytes per pixel.
	int getBytesPerPixel(void) const;

	//! Copy the pixels into a new image buffer.
	void reserveImage(int width, int height, ColorMode mode);

    //! Cut from an original sprit
    //! x_start: x start position of original sprit for cutting
    //! y_start: y start position of original sprit for cutting
    //! widht:   cut width
    //! height:  cut height
    void cut(const Sprite& original, int x_start, int y_start, int width, int height);

	//! Load the image from a bitmap file.
    // @brief Check file signature and load PNG or BMP file.
	ReturnCode loadImage(const FileNameCharType* file_path, bool bIsGray=false);

    //! Load the image from a bitmap file.
    // @brief Check file signature and load PNG or BMP file.
    ReturnCode loadImage(const FileName& file_path);

	//! Load the image from a bitmap buffer.
	// @brief Check file signature and load PNG or BMP file.
    ReturnCode loadFromStream(unsigned char* data, const unsigned int dataSize);
    //! Save the image to a bitmap file.
	ReturnCode saveImage(const FileNameCharType* file_path);

    //! Save the image to a bitmap file.
    ReturnCode saveImage(const FileName& file_path);
	//! Get PNG format file buffer from a sprite
	ReturnCode getPngFileBuffer(AutoSharedPtr<unsigned char>& buffer, unsigned int& bufferSize, bool bCompression = true);
	//! Get JPG format file buffer from a sprite
	ReturnCode getJpgFileBuffer(AutoSharedPtr<unsigned char>& buffer, unsigned int& bufferSize, int quality = 80);
	//! flip the image buffer indeed.
	void ExchangeLine(int iWidth, int iHigh);

	//! Load the image from a raw image buffer
	ReturnCode loadImageBuffer(unsigned char * pBuffer, unsigned int width, unsigned int height, ColorMode mode, bool flippedFlag);

	//! return m_Flipped, 0 represent it is not flipped vertically, non-zero means it is flipped vertically
	bool isFlipped()const;
	void setVerticalOrientation(bool inverted);

	//! size of color table must less than 256
	void setPngColorPalette(const AutoArray<PAL_ENTRY_COLOR>& colorTable);
	//! update alpha when ColorModeRGBA32 or ColorModeBGRA32
	void updateTranspancy(const unsigned char alpha);
    void resetPngColorPalette();
	void EnablePalette(bool flag){m_NeedPalette=flag;}

    void setPngCompressLevel(int level) {m_PngCompressLevel = level;}
    int  getPngCompressLevel(void) {return m_PngCompressLevel;}

private:
	//!flip vertically
	void Flip();
private:
	int             m_Width;
	int             m_Height;
	ColorMode       m_ColorMode;
	AutoSharedPtr<unsigned char> m_ImageBuffer;
	int             m_BufferSize;
	FileName        m_FileName;
	//it represents whether the image of sprite is flipped.
	//the default value of m_Flipped is FLIPPED.
	//Lazy write policy is used here, we don't do the flipping until we output the image.
	bool	m_Flipped;

	//use palette or not by this flag
	bool	m_NeedPalette;

	//png file color palette
	AutoArray<PAL_ENTRY_COLOR> m_ColorTable;

    // png compression level
    int m_PngCompressLevel;

    ReturnCode readJPG(FILE* fpr);
	bool isNeedPalette(ColorMode colorMode);
};

#ifdef _MSC_VER
template class NCDB_API AutoArray <Sprite>;
#endif

};
#endif // SPRITE_H_ONCE
/*! @} */

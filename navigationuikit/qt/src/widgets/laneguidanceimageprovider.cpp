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

#include "laneguidanceimageprovider.h"
#include "navigationuikitwrapper.h"
#include <QPainter>
#include <QImage>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>

#define TRUNC(x) ((x) >> 6)
#define FONT_SIZE  0
#define IMG_SIZE 4
#define BACK_COLOR 2
#define FOGE_COLOR 1
#define CHAR  3
#define  FONTSIZE_SCALE 64

using namespace locationtoolkit;
LaneguidanceImageProvider::LaneguidanceImageProvider()
    : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
{
    FT_Error error = FT_Err_Ok;
    mFace = 0;
    mLibrary = 0;
    QString root = NavigaitonUIKitWrapper::GetResourcePath();
    QString fontFilePath = root + "/font/LG_NexGen.ttf";
    error = FT_Init_FreeType(&mLibrary);
    if (!error)
    {
        FT_New_Face(mLibrary, fontFilePath.toLatin1().data(), 0, &mFace);
    }
}

QPixmap LaneguidanceImageProvider::requestPixmap(const QString &id, QSize* size, const QSize &requestedSize)
{
    int fontSize = 50;  //default size
    int picSize = 65;
    QChar character = 0;
    QStringList list = id.split("*");
    if(list.size() < 5)  //the id have 5 item(size,backgraound color, foreground color, char, image size)
    {
        if(size)
        {
            *size = QSize(picSize,picSize);
        }
        QPixmap pixmap(picSize, picSize);
        return pixmap;
    }
    fontSize = list[FONT_SIZE].toInt();
    picSize = list[IMG_SIZE].toInt();
    QColor background(list[BACK_COLOR]);
    QColor foreground(list[FOGE_COLOR]);
    if(list[CHAR].size() > 0)
    {
        character = list[CHAR][0];
    }
    if(size)
    {
        *size = QSize(picSize,picSize);
    }
    QPixmap pixmap(picSize, picSize);
    pixmap.fill(background.rgba());
    FT_Error error;
    if (mFace)
    {
        QPainter painter(&pixmap);
        error = FT_Set_Char_Size(mFace, 0,  (fontSize) * FONTSIZE_SCALE,  pixmap.physicalDpiX(), pixmap.physicalDpiY());
        if (!error)
        {
            FT_UInt glyph_index = 0;
            glyph_index = FT_Get_Char_Index(mFace, character.unicode());
            error = FT_Load_Glyph(mFace, glyph_index,  FT_LOAD_DEFAULT);
            if (!error)
            {
                FT_Pos left = mFace->glyph->metrics.horiBearingX;
                FT_Pos right = left + mFace->glyph->metrics.width;
                FT_Pos top = mFace->glyph->metrics.horiBearingY;
                FT_Pos bottom = top - mFace->glyph->metrics.height;
                QRect rect;
                rect = QRect(QPoint(TRUNC(left),  -TRUNC(top) + 1), QSize(TRUNC(right - left) + 1, TRUNC(top - bottom) + 1));
                FT_Render_Glyph(mFace->glyph,  FT_RENDER_MODE_NORMAL);

                QImage glyphImage(mFace->glyph->bitmap.buffer,
                                  mFace->glyph->bitmap.width,
                                  mFace->glyph->bitmap.rows,
                                  mFace->glyph->bitmap.pitch,
                                  QImage::Format_Indexed8);

                QVector<QRgb> colorTable;
                for (int i = 0; i < 256; ++i)
                {
                    colorTable << qRgba(foreground.red(), foreground.green(), foreground.blue(), i);
                }
                glyphImage.setColorTable(colorTable);
                int x = (picSize - rect.width()) /2;
                int y = (picSize - rect.height()) /2;
                painter.drawImage(QPoint(x, y),glyphImage);
            }
        }
    }
    return pixmap;
}

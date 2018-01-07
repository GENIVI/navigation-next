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

#ifndef SEARCHTEST_H
#define SEARCHTEST_H

#include <Session.h>
#include "searchparam.h"
#include <QDateTime>
#include <QDebug>
#include <iostream>

class ElapseTime
{
public:
    ElapseTime(const QString& msg, const char *pre = "Enter ", const char *post = "Exit ") : m_msg(msg), m_post(post) ,m_start(QDateTime::currentDateTime()){
        //qDebug() << pre << m_msg << " at: " << m_start.toString(Qt::RFC2822Date);
        std::cout << pre << m_msg.toStdString() << " at: " << m_start.toString(Qt::RFC2822Date).toStdString() << std::endl;
    }


    ~ElapseTime() {
        QDateTime end(QDateTime::currentDateTime());
        qint64 elapse = end.toMSecsSinceEpoch() - m_start.toMSecsSinceEpoch();
        std::cout << m_post << m_msg.toStdString() << " at: " <<  end.toString(Qt::RFC2822Date).toStdString() << " for an elapse of " << QString::number(elapse).toStdString() << " ms" << std::endl;
    }

private:
    QString m_msg;
    const char *m_post;
    QDateTime m_start;
};

class SearchTest
{

public:
    explicit SearchTest();
    ~SearchTest();

public:
    void DoSearch(const SearchParam& param);
    void DoReverseGeocode();
    void PrintMapDataVersion();

private:
    Ncdb::ReturnCode OpenIni(const char* path);
    Ncdb::ReturnCode OpenWM(const char* path);
    Ncdb::ReturnCode AddPath(const char* path);
    Ncdb::ReturnCode OpenConfig(const char* path);
    void cout(const std::string& buf);

private:
    Ncdb::Session m_ncdbSession;
    bool m_poiInited;
    bool m_geocodeInited;
};

#endif // SEARCHTEST_H

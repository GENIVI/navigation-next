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
  @file        OnboardRouter.cpp
  @defgroup    navigation
*/
/*
  (C) Copyright 2015 by TeleCommunication Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.
  --------------------------------------------------------------------------*/

/*! @{ */

#include "useonboard.h"

#include "OnboardRouter.h"
#include "OnboardRouteHandler.h"
#include "nbnaturallaneguidancetype.h"
#include "ncdbobjectsmanager.h"
#include "workerqueuemanager.h"
#include "StringUtility.h"
#include "Route.h"
#include "RouteManager.h"
#include "data_via_point.h"
//#define MPERF

#ifdef MPERF
#include <QDateTime>
#include <QDebug>
#include <pthread.h>
#endif

extern "C"
{
#include "palclock.h"
#include "nbspatial.h"
#include "nbrouteparametersprivate.h"
#include "data_lane_guidance_item.h"
#include "data_speed_region.h"
#include "data_gps.h"
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
}

#include <functional>
#include <math.h>
#include <algorithm>

typedef struct
{
    uint32             dwTimeStamp;          // Time, seconds since 1/6/1980
    int32              dwLat;                // Lat, 180/2^25 degrees, WGS-84 ellipsoid
    int32              dwLon;                // Lon, 360/2^26 degrees, WGS-84 ellipsoid
    uint16             wHeading;             // Heading, 360/2^10 degrees
    int16              wVelocityHor;         // Horizontal velocity, 0.25 meters/second
} PackedGps;

static NB_NavigateRouteError ConvertNcdbToNbErrors(Ncdb::ReturnCode);
static NB_NavigateRouteError ConvertNbErrorsToRouteError(NB_Error);
static NB_Error ConvertRouteSummary(NB_Context* context, Ncdb::FastETA& eta, CSL_Vector* routeSummarys);
static NB_Error ConvertNavReply(NB_Context*                   context,
                                NB_RouteParameters*           params,
                                Ncdb::AutoArray<Ncdb::Route>& routes,
                                const std::vector<string> &routeIDs,
                                const Ncdb::WorldRect&        box,
                                data_nav_reply*               pnr);


#define MAX(X, Y)      (X) < (Y) ? (Y) : (X)
#define MIN(X, Y)      (X) < (Y) ? (X) : (Y)
#define SET_MIN(X, Y)  (X) = MIN(X, Y)
#define SET_MAX(X, Y)  (X) = MAX(X, Y)
#define CHECK_ERROR(err)                        \
    if (err != NE_OK)                           \
    {                                           \
        return err;                             \
    }


#define APPEND_DATA_POINTER(V, P, ERR)          \
    do                                          \
    {                                           \
        if (!CSL_VectorAppend((V), (P)))        \
        {                                       \
            (ERR) = NE_NOMEM;                   \
        }                                       \
    } while (0)

#define TO_NB_BOOL(X)       (X) ? TRUE : FALSE
#define FROM_NB_BOOL(X)     ((X) == TRUE) ? true : false

#define POLYLINE_INVALID_HEADING -999.0
#define  MPS_MPH                 2.2369362920543878
#define  MPS_KPH                 3.6
#define MAX_DISTANCE  50
#define ROUTE_ID_LENGTH 48
#define STRING_BUF_SIZE 64

namespace nbnav
{

class OnboardRouteParameters
{
public:
    friend class OnboardRouter;
    OnboardRouteParameters(OnBoardRouteRequest request);
    ~OnboardRouteParameters();

protected:
    enum RouteCalculateType {
        RCT_Unknown,
        RCT_FromPlace ,
        RCT_FromHistory,
        RCT_Detour,
        RCT_OffRoute,
        RCT_Recreate,
        RCT_SingleRoute,
        RCT_MultiRoute
    };

    float                 m_heading;
    Ncdb::WorldPoint      m_start;
    Ncdb::WorldPoint      m_end;
    Ncdb::WorldPointList  m_viaPoints;
    Ncdb::RoutingOptions  m_routeOption;
    Ncdb::ManeuverOptions m_maneuverOption;
    Ncdb::WorldRect       m_routeBoundBox;
    bool                  m_ready;
    RouteCalculateType    m_calroutetype;
    OnBoardRouteRequest   m_request;
    bool                  m_canceled;
    int                   m_destStreetNumber;

    Ncdb::AutoArray<Ncdb::FastETA> m_extraDestination;
    bool                  m_wantRouteSummary;
    std::string           m_activeRouteID;
    Ncdb::RoadSegmentIDList m_roadSegment;
    float                 m_speed;

private:
    void SetupRouteOptions(data_nav_query* query, data_util_state* pds);
    void SetupManeuverOptions(data_nav_query* query, data_util_state* pds);
    void SetupCalculateType(data_nav_query* query, data_util_state* pds);
    void SetupViaPoints(data_nav_query* query, data_util_state* pds);
    NB_DISABLE_COPY(OnboardRouteParameters)
};

class RouteRequestResult
{
public:
    RouteRequestResult(Ncdb::ReturnCode          calcResult,
                       OnboardRouteParametersPtr parameters);
    virtual ~RouteRequestResult();

    Ncdb::ReturnCode             m_calcResult;
    Ncdb::WorldRect              m_box;
    Ncdb::AutoArray<Ncdb::Route> m_routes;
    Ncdb::AutoArray<Ncdb::FastETA> m_fastETAList;

    OnboardRouteParametersPtr    m_parameters;
    std::vector<std::string>     m_routeIDs;
};

template<class T>
class OnboardNavigationTask : public Task
{
public:
    OnboardNavigationTask(OnboardRouter& router,
                          shared_ptr<T> params,
                          void (OnboardRouter::*func)(shared_ptr<T>))
        : m_executer(func),
          m_rRouter(router),
          m_pParams(params) {}
    virtual ~OnboardNavigationTask() {}
    virtual void Execute()
    {
        m_executer(m_rRouter, m_pParams);
        delete this;
    }
private:
    mem_fun1_ref_t<void, OnboardRouter, shared_ptr<T> > m_executer;
    OnboardRouter&            m_rRouter;
    shared_ptr<T> m_pParams;
};
}

using namespace nbnav;
using namespace nbcommon;

static NB_Error GenerateDetourSegementList(NB_RouteParameters* parameters, const Ncdb::Route& currentRoute, Ncdb::RoadSegmentIDList& detourSegs, Ncdb::WorldPoint& startPoint)
{
    const double KWGSCoefficient    = 0.00000536441803;
    NB_Error err = NE_OK;
    if(!parameters)
    {
        return NE_INVAL;
    }
    //get start point gps
    double latitude = 0;
    double logitude = 0;
    detourSegs.clear();
    if(CSL_VectorGetLength(parameters->dataQuery.origin.vec_gps) > 0)
    {
        data_gps* gps = (data_gps*)CSL_VectorGetPointer(parameters->dataQuery.origin.vec_gps, 0);
        PackedGps* pkGps = (PackedGps*)gps->packed;
        latitude = ((int32)nsl_ntohl(pkGps->dwLat))*KWGSCoefficient;
        logitude = ((int32)nsl_ntohl(pkGps->dwLon))*KWGSCoefficient;
        startPoint.x = logitude;
        startPoint.y = latitude;
    }
    //find the nearst polyline point on route
    uint32 index = 0;
    double maxDistance = -1;
    const Ncdb::WorldPointList& polyline = currentRoute.GetRoutePolyline();
    for(int i = 0; i< polyline.size(); i++)
    {
        double heading;
        double distance = NB_SpatialGetLineOfSightDistance(latitude, logitude, polyline[i].y, polyline[i].x, &heading);
        if(maxDistance < 0 || maxDistance > distance)
        {
            maxDistance = distance;
            index = i;
        }
    }
    //get route segementid list
    int number = currentRoute.GetNoOfRoadSegmentsinRoute();
    for(int i = 0; i< number; i++)
    {
        const Ncdb::RouteSegment& segment = currentRoute.GetRouteSegment(i);
        if(segment.m_indexNormalPolyStart >= index)
        {
            detourSegs.add(segment.m_SegmentID);
        }
    }
    return err;
}

static PAL_Error GenerateRouteID(PAL_Instance* instance, std::string& id)
{
    uint8 uuid[ROUTE_ID_LENGTH] = {0};
    uint32 length = 0;
    PAL_Error err = PAL_GenerateUUID(instance, uuid, ROUTE_ID_LENGTH, &length);
    if(err == PAL_Ok)
    {
        id = std::string((char*)uuid, length);
    }
    return err;
}

static const  Ncdb::WorldRect DefaultBoundBox = {200.0, -100.0, -200.0, 100.0};

// Implementation of OnboardRouter.

/* See description in header file. */
OnboardRouter::OnboardRouter(NB_Context* context)
    : m_pContext(context),
      m_initialized(false)
{
    PAL_LockCreate(NB_ContextGetPal(context), &m_lock);
}

/* See description in header file. */
OnboardRouter::~OnboardRouter()
{
    PAL_LockDestroy(m_lock);
    //XXX: Implement this!
}

#define MAX_RETRY_GET_NCDB_SESSION 30

/* See description in header file. */
void OnboardRouter::Initialize()
{
#ifdef MPERF
    qWarning() << "NCDB" << pthread_self() << "Enter Onboard Router Initialize()";
#endif
    if (!m_pContext)
    {
        return;
    }

    m_pWorkTaskQueue = ContextBasedSingleton<WorkerQueueManager>::getInstance(
           m_pContext)->RetrieveCommonWorkerTaskQueue();
    m_pEventTaskQueue = shared_ptr<EventTaskQueue>(CCC_NEW EventTaskQueue(NB_ContextGetPal(m_pContext)));

    /* give ncdb session 30 seconds for initialization -- reading data base */
    int count = 0;
    m_pSession = ContextBasedSingleton<NcdbObjectsManager>::getInstance(m_pContext)->GetNcdbSession();
    while (!m_pSession && count < MAX_RETRY_GET_NCDB_SESSION)
    {
#ifdef Q_OS_UNIX
        sleep(1);
#endif
        count++;
        m_pSession = ContextBasedSingleton<NcdbObjectsManager>::getInstance(m_pContext)->GetNcdbSession();
    }

    // check and set m_ready
    if (m_pSession && m_pSession->IsOpened())
    {
        m_pRouteManager.reset(CCC_NEW Ncdb::RouteManager(*m_pSession));
        m_pSession->OpenHistoricTraffic();
        
        m_initialized = m_pRouteManager.get() != NULL;
    }
#ifdef MPERF
    qWarning() << "NCDB" << pthread_self() << "Exit Onboard Router Initialize()";
#endif
}

/* See description in header file. */
NB_Error OnboardRouter::StartRouteRequest(OnBoardRouteRequest  request)
{
    m_currRequestParam.reset(CCC_NEW OnboardRouteParameters(request));

    if(m_currRequestParam->m_calroutetype == OnboardRouteParameters::RCT_Detour)
    {
        int index = -1;
        PAL_LockLock(m_lock);
        for (size_t i = 0; i < m_currentRouteIDs.size(); i++)
        {
            if (!(nsl_strcmp(m_currentRouteIDs[i].c_str(),m_currRequestParam->m_activeRouteID.c_str())))
            {
                index = i;
                break;
            }
        }
        if(index >= 0)
        {
            GenerateDetourSegementList(m_currRequestParam->m_request.parameters, m_currentRoutes[index],
                                       m_currRequestParam->m_roadSegment, m_currRequestParam->m_start);
            PAL_LockUnlock(m_lock);
        }
        else
        {
            PAL_LockUnlock(m_lock);
            RouteRequestResultPtr result(CCC_NEW RouteRequestResult( Ncdb::NCDB_NO_ALTERNATE,m_currRequestParam));
            CCCThread_RequestComplete(result);
            return NE_OK;
        }
    }

    PAL_Error error =  m_pWorkTaskQueue->AddTask(
        CCC_NEW OnboardNavigationTask<OnboardRouteParameters>(
            *this, m_currRequestParam,
            &OnboardRouter::WorkerThread_StartRouteRequest));

    // add task to worker thread.
    return error == PAL_Ok ? NE_OK : NE_UNEXPECTED;
}

void OnboardRouter::CancelRouteRequest()
{
    if(m_currRequestParam)
    {
        m_currRequestParam->m_canceled = true;
    }
}

void OnboardRouter::WorkerThread_StartRouteRequest(OnboardRouteParametersPtr params)
{
    if (!params)
    {
        // todo: check if we need to notify upper layer about this..
        return;
    }
    if(params->m_canceled)
    {
        return;
    }


    if(!m_pRouteManager || !m_initialized)
    {
        ScheduleErrorCallback(params, Ncdb::NCDB_OPEN_ERR);
        return;
    }

    m_pRouteManager->SetManeuverOptions(params->m_maneuverOption);

    //calculate route
    if(params->m_start.x == -999.0f && params->m_start.y == -999.0f)
    {
        ScheduleErrorCallback(params, Ncdb::NCDB_ORIGIN_NOT_ACCESSIBLE);
        return;
    }

    m_pRouteManager->SetTimeLimit(100000); //@todo: should be set to a valid value.

    RouteRequestResultPtr result(CCC_NEW RouteRequestResult(Ncdb::NCDB_OK, params));

#ifdef MPERF
    qint64 startTime;
    qint64 endTime;
#endif
    
    switch(params->m_calroutetype)
    {
        case OnboardRouteParameters::RCT_Detour:
        {
#ifdef MPERF
            startTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager begin Detour calculate route at"<<startTime;
#endif
            
            int index = -1;
            PAL_LockLock(m_lock);
            for (int i = 0; i < m_currentRouteIDs.size(); i++)
            {
                if (!(nsl_strcmp(m_currentRouteIDs[i].c_str(),params->m_activeRouteID.c_str())))
                {
                    index = i;
                    break;
                }
            }
            if(index >= 0)
            {
                result->m_calcResult = m_pRouteManager->CalculateDetourRoutes(m_currentRoutes[index],
                                                                params->m_roadSegment,
                                                                params->m_start,
                                                                params->m_heading,
                                                                params->m_speed, result->m_routes);
                 //Remove the first route from routes.
                if (result->m_routes.size() == 1)
                {
            result->m_calcResult = Ncdb::NCDB_NO_ALTERNATE;
                }
                else
                {
                    Ncdb::AutoArray<Ncdb::Route> tempRoutes = result->m_routes;
                    result->m_routes.clear();
                    for (int i = 1; i < tempRoutes.size(); i++)
                    {
                        result->m_routes.push_back(tempRoutes[i]);
                    }
                }
            }
            else
            {
                result->m_calcResult = Ncdb::NCDB_NO_ALTERNATE;
            }
            PAL_LockUnlock(m_lock);
            
#ifdef MPERF
            endTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager finish Detour calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif

            
            break;
        }
        case OnboardRouteParameters::RCT_OffRoute:

        {

#ifdef MPERF
        startTime = QDateTime::currentMSecsSinceEpoch();
        qWarning()<<"MPERF RouteManager begin ReturnToRoute calculate route at"<<startTime;
#endif

            int index = -1;
            for (int i = 0; i < m_currentRouteIDs.size(); i++)
            {
                if (!(nsl_strcmp(m_currentRouteIDs[i].c_str(),params->m_activeRouteID.c_str())))
                {
                    index = i;
                    break;
                }
            }

            if(index >= 0) {
                Ncdb::Route recalcRoute;
                recalcRoute.SetDestAddressNumber(params->m_destStreetNumber);
                result->m_calcResult = m_pRouteManager->ReturnToRoute(m_currentRoutes[index],
                                                                      params->m_start,
                                                                      params->m_heading,
                                                                      params->m_speed,
                                                                      recalcRoute);

                result->m_routes.add(recalcRoute);
            } else {
                //Should not happen
                result->m_calcResult = Ncdb::NCDB_NO_ALTERNATE;
            }

#ifdef MPERF
            endTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager finish ReturnToRoute calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif

            break;
        }

        case OnboardRouteParameters::RCT_SingleRoute:
        {
#ifdef MPERF
            startTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager begin SingleRoute calculate route at"<<startTime;
#endif
            
            Ncdb::Route singleRoute;
            singleRoute.SetDestAddressNumber(params->m_destStreetNumber);
            if(params->m_viaPoints.size()>0)
            {
                result->m_calcResult = m_pRouteManager->CalculateRoutePassingThroughIntermediatePoints(params->m_routeOption,
                                                                params->m_start,
                                                                params->m_end,
                                                                params->m_viaPoints,
                                                                params->m_heading,
                                                                params->m_speed, singleRoute);
            }
            else
            {
            result->m_calcResult = m_pRouteManager->CalculateRoute(params->m_routeOption,
                                                         params->m_start,
                                                         params->m_end,
                                                         params->m_heading,
                                                         params->m_speed,
                                                         singleRoute);
            }
            result->m_routes.add(singleRoute);
#ifdef MPERF
            endTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager finish SingleRoute calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif
            
            break;
        }
        default:
        {
#ifdef MPERF
            startTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager begin MultiRoutes calculate route at"<<startTime;
#endif
            
            Ncdb::Route singleRoute;
            singleRoute.SetDestAddressNumber(params->m_destStreetNumber);
            result->m_routes.add(singleRoute);
            if(params->m_viaPoints.size()>0)
            {
                result->m_calcResult = m_pRouteManager->CalculateRoutePassingThroughIntermediatePoints(params->m_routeOption,
                                                                params->m_start,
                                                                params->m_end,
                                                                params->m_viaPoints,
                                                                params->m_heading,
                                                                params->m_speed, singleRoute);
            }
            else
            {
            result->m_calcResult = m_pRouteManager->CalculateMultiRoutes(params->m_routeOption,
                                                               params->m_start,
                                                               params->m_end,
                                                               params->m_heading,
                                                               params->m_speed,
                                                               result->m_routes);
            }
#ifdef MPERF
            endTime = QDateTime::currentMSecsSinceEpoch();
            qWarning()<<"MPERF RouteManager finish MultiRoutes calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif
        }
    }

    if (result->m_calcResult == Ncdb::NCDB_OK)
    {
        PAL_LockLock(m_lock);
        for (int i = 0; i < result->m_routes.size(); i++)
        {
            std::string routeID;
            GenerateRouteID(NB_ContextGetPal(m_pContext), routeID);
            result->m_routeIDs.push_back(routeID);
        }
        for (int i = 0; i < result->m_routes.size(); i++)
        {
            Ncdb::WorldRect box = DefaultBoundBox;
            m_pRouteManager->CalculateRoutePolylineBoundingBox(result->m_routes[i], box);

            // merge two boxes..
            SET_MIN(result->m_box.m_Left, box.m_Left);
            SET_MAX(result->m_box.m_Top, box.m_Top);
            SET_MAX(result->m_box.m_Right, box.m_Right);
            SET_MIN(result->m_box.m_Bottom, box.m_Bottom);
        }
        if(params->m_wantRouteSummary && params->m_extraDestination.size()>0)
        {
            Ncdb::RouteStartTime startTime;
            PAL_ClockDateTime palTime;
            PAL_ClockGetDateTime(&palTime);
            startTime.SetTime(palTime.hour + palTime.minute/60,
                              palTime.dayOfWeek, palTime.day, palTime.month, palTime.year);
            result->m_calcResult = m_pRouteManager->CalculateFastETA(params->m_routeOption,
                                                           params->m_start, params->m_heading,
                                                           0, startTime, params->m_extraDestination, MAX_DISTANCE);

            result->m_fastETAList = params->m_extraDestination;
        }

        if(params->m_calroutetype == OnboardRouteParameters::RCT_Detour)
        {
            int index = -1;
            for (int i = 0; i < m_currentRouteIDs.size(); i++)
            {
                if (!(nsl_strcmp(m_currentRouteIDs[i].c_str(),params->m_activeRouteID.c_str())))
                {
                    index = i;
                    break;
                }
            }
            if(index >= 0)
            {
                m_currentRouteIDs.clear();
                m_currentRouteIDs = result->m_routeIDs;
                m_currentRouteIDs.push_back(params->m_activeRouteID);
                Ncdb::Route route = m_currentRoutes[index];
                m_currentRoutes.clear();
                m_currentRoutes = result->m_routes;
                m_currentRoutes.push_back(route);
            }
        }
        else
        {
            m_currentRouteIDs.clear();
            m_currentRouteIDs = result->m_routeIDs;
            m_currentRoutes.clear();
            m_currentRoutes = result->m_routes;
        }
        PAL_LockUnlock(m_lock);
    }

    m_pEventTaskQueue->AddTask(CCC_NEW OnboardNavigationTask<RouteRequestResult>
                          (*this, result,
                           &OnboardRouter::CCCThread_RequestComplete));
}

/* See description in header file. */
void OnboardRouter::CCCThread_RequestComplete(RouteRequestResultPtr result)
{
    if (result && result->m_parameters && !result->m_parameters->m_canceled)
    {
        OnBoardRouteRequest* request = &result->m_parameters->m_request;
        data_util_state* pds = NB_ContextGetDataState(request->parameters->context);
        data_nav_reply* reply = (data_nav_reply*)malloc(sizeof(data_nav_reply));
        memset(reply, 0, sizeof(data_nav_reply));
        data_nav_reply_init(pds,reply);
        NB_NavigateRouteError routeError = ConvertNcdbToNbErrors(result->m_calcResult);
        NB_Error err = NE_OK;
        if(routeError == NB_NRE_None)
        {
            err = ConvertNavReply(request->parameters->context,
                                  request->parameters,
                                  result->m_routes,
                                  result->m_routeIDs,
                                  result->m_box,
                                  reply);
            if(err == NE_OK)
            {
                for(int i = 0; i<result->m_fastETAList.size(); i++)
                {
                    ConvertRouteSummary(request->parameters->context, result->m_fastETAList[i], reply->route_summarys);
                }
            }
        }
#ifdef MPERF
        qint64 startTime = QDateTime::currentMSecsSinceEpoch();
        qWarning()<<"MPERF RouteManager begin OnBoardRouterResult calculate route at"<<startTime;
#endif

        request->requestCB(routeError, err, reply, request->userData);
        
#ifdef MPERF
        qint64 endTime = QDateTime::currentMSecsSinceEpoch();
        qWarning()<<"MPERF RouteManager finish OnBoardRouterResult calculate route at"<<endTime<<", used time="<<endTime - startTime;
#endif
        
        data_nav_reply_free(pds,reply);
        free(reply);
    }
}

/* See description in header file. */
inline void OnboardRouter::ScheduleErrorCallback(OnboardRouteParametersPtr params,
                                                 Ncdb::ReturnCode calcResult)
{
    RouteRequestResultPtr result(CCC_NEW RouteRequestResult(calcResult, params));
    m_pEventTaskQueue->AddTask(CCC_NEW OnboardNavigationTask<RouteRequestResult>
                          (*this, result,
                           &OnboardRouter::CCCThread_RequestComplete));
}

static inline void DataPointToWorldPoint(const data_point& from, Ncdb::WorldPoint& to)
{
    to.Set(from.lon, from.lat);
}

// Implementation of OnboardRouteParameters. This class is used internally, all
// arguments should have been checked before, and we don't need to check arguments
// again.

/* See description in header file. */
OnboardRouteParameters::OnboardRouteParameters(OnBoardRouteRequest request)
    : m_calroutetype(RCT_Unknown),
      m_request(request),
      m_canceled(false),
      m_destStreetNumber(-1)
{
    const float KSpeedScale = 0.25;
    NB_RouteParameters* parameters = m_request.parameters;
    data_nav_query* query  = &parameters->dataQuery;

    if(parameters->originFix.longitude == -999.0f || parameters->originFix.latitude == -999.0f)
    {
        DataPointToWorldPoint(query->origin.point, m_start);
    }
    else
    {
        m_start.Set(parameters->originFix.longitude, parameters->originFix.latitude);
    }
    DataPointToWorldPoint(query->destination.point, m_end);
    m_heading = parameters->originFix.heading;
    m_destStreetNumber = nsl_atoi(query->destination.address.sa);

    // @todo: set actual heading...
    data_util_state* pds = NB_ContextGetDataState(parameters->context);
    SetupRouteOptions(query, pds);
    SetupManeuverOptions(query, pds);
    SetupCalculateType(query, pds);
    SetupViaPoints(query, pds);

    m_activeRouteID = std::string((const char*)query->recalc.route_id.data, query->recalc.route_id.size);
    if(query && query->origin.vec_gps && CSL_VectorGetLength(query->origin.vec_gps) > 0)
    {
        data_gps* gps = (data_gps*)CSL_VectorGetPointer(query->origin.vec_gps,0);
        PackedGps* pkGps = (PackedGps*)gps->packed;
        if(pkGps->wVelocityHor > 0)
        {
            m_speed = ntohs(pkGps->wVelocityHor)*KSpeedScale;
        }
        else
        {
            m_speed = 0;
        }
    }
    else
    {
        m_speed = 0;
    }
    if(nsl_strcmp(request.parameters->language,"zh-CN") == 0)
    {
        m_routeOption.SetUserLanguage(Ncdb::Chinese_Simplified);
    }
    else if(nsl_strcmp(request.parameters->language,"fr-CA") == 0)
    {
        m_routeOption.SetUserLanguage(Ncdb::French);
    }
    else if(nsl_strcmp(request.parameters->language,"en-US") == 0)
    {
        m_routeOption.SetUserLanguage(Ncdb::US_English);
    }
}

/* See description in header file. */
OnboardRouteParameters::~OnboardRouteParameters()
{
    //XXX: Implement this!
}

#define ARRAY_SIZE(X)       (int)(sizeof(X)/sizeof(X[0]))

typedef struct _OptimizeTypeMapping
{
    const char*             description;
    Ncdb::RouteOptimization optimization;
} OptimizeTypeMapping;

static OptimizeTypeMapping optimizeMappings[] =
{
    {"fastest",  Ncdb::RO_Fastest},
    {"shortest", Ncdb::RO_Shortest},
    {"easiest",  Ncdb::RO_Easiest},
};

typedef struct _VehicleTypeMapping
{
    const char*       description;
    Ncdb::VehicleType vehicleType;
} VehicleTypeMapping;

static VehicleTypeMapping vehicleMappings[] =
{
    {"car",         Ncdb::Car},
    {"truck",       Ncdb::Truck},
    {"bicycle",     Ncdb::Bicycle},
    {"pedestrian",  Ncdb::Pedestrian},
};

/* See description in header file. */
void
OnboardRouteParameters::SetupRouteOptions(data_nav_query* query,
                                          data_util_state* pds)
{
    //set avoids
    data_route_style* style = &query->route_style;

    m_routeOption.SetAvoidFerry(style->avoid_ferry);
    m_routeOption.SetAvoidUnpavedRoads(style->avoid_unpaved);
    m_routeOption.SetAvoidUturn(style->avoid_uturn);
    m_routeOption.SetAvoidHwy(style->avoid_highway);
    m_routeOption.SetAvoidToll(style->avoid_toll);
    m_routeOption.SetAvoidHOV(style->avoid_hov);

    //set traffic, does not support for now..
    if(0)
    {
        m_routeOption.SetUseRealTraffic(query->want_realtime_traffic);
        m_routeOption.SetUseHistoricalTraffic(query->want_historical_traffic);
    }

    //set route optimization
    Ncdb::RouteOptimization optimization = Ncdb::RO_Fastest;
    const char* routeType = data_string_get(pds, &style->optimize);
    if (routeType)
    {
        for (int i = 0; i < ARRAY_SIZE(optimizeMappings); i++) {
            if (!nsl_strcmp(optimizeMappings[i].description, routeType))
            {
                optimization = optimizeMappings[i].optimization;
                break;
            }
        }
    }
    m_routeOption.SetRouteOptimization(optimization);

    //set vehicle type
    Ncdb::VehicleType vehicle = Ncdb::Car;
    const char* vehicleType = data_string_get(pds, &style->vehicle_type);
    if (vehicleType)
    {
        for (int i = 0; i < ARRAY_SIZE(vehicleMappings); i++) {
            if (!nsl_strcmp(vehicleMappings[i].description, vehicleType))
            {
                vehicle = vehicleMappings[i].vehicleType;
                break;
            }
        }
    }
    m_routeOption.SetVehicleType(vehicle);

    m_wantRouteSummary = query->want_route_summary;
    Ncdb::FastETA place;
    place.m_loc.Set(query->destination.point.lon, query->destination.point.lat);
    m_extraDestination.add(place);
    uint32 length = CSL_VectorGetLength(query->extra_destination);
    for(size_t i = 0; i < length; i++)
    {
        data_destination* dest = (data_destination*)CSL_VectorGetPointer(query->extra_destination, i);
        place.m_loc.Set(dest->point.lon, dest->point.lat);
        m_extraDestination.add(place);
    }
}

/* See description in header file. */
void OnboardRouteParameters::SetupManeuverOptions(data_nav_query* query,
                                                  data_util_state* /*pds*/)
{
    m_maneuverOption.SetManeuverOptions(false, //turn map
                                        query->want_unpaved_info,
                                        false, // simple manuever
                                        query->want_bridge_maneuvers,
                                        query->want_ferry_maneuvers,
                                        query->want_tunnel_maneuvers,
                                        query->want_enter_country_maneuvers,
                                        query->want_country_info,
                                        query->want_cross_streets,
                                        false, // route polyline, @todo: check this.
                                        query->want_enter_roundabout_maneuvers);

    m_maneuverOption.SetCmdSetVersion(query->command_set_version);
    m_maneuverOption.SetPronunWnd(query->max_pronun_files);
    m_maneuverOption.SetEnNVOptions(query->want_realistic_road_signs,
                                    query->want_junction_models,
                                    query->want_city_models);

    m_maneuverOption.SetSpeedLimitedOption(query->want_speed_regions);

    data_want_guidance_config* guidanceConfig = &query->want_guidance_config;
    m_maneuverOption.SetWantLaneGuidance(guidanceConfig->want_lane_guidance);
    m_maneuverOption.SetWantNaturalGuidance(guidanceConfig->want_natural_guidance);
//    m_maneuverOption.SetWantEnhancedNaturalGuidance(
//        guidanceConfig->want_enchanced_natural_guidance);
    /*@TODO:NCDB has split the natural guidance into bridge and traffic light,
            should the client changes the same as NCDB? */
    m_maneuverOption.SetWantTrafficLights(guidanceConfig->want_natural_guidance);

    //To support M1. feature.
    m_maneuverOption.SetWantFirstManeuverCrossroad(query->want_first_maneuver_cross_streets);
    m_maneuverOption.SetWantExitNumber(query->want_exit_numbers);
    m_maneuverOption.SetRoutePolyWnd(0);
}

/* See description in header file. */
void OnboardRouteParameters::SetupCalculateType(data_nav_query*  query,
                                                data_util_state* /*pds*/)
{
    m_calroutetype = RCT_Unknown;
    // @todo: fill more correct types...
    if (query->want_alternate_routes.max_routes < 2)
    {
        m_calroutetype = RCT_SingleRoute;
    }
    else
    {
        m_calroutetype = RCT_MultiRoute;
    }
    if (query->recalc.why)
    {
        if(nsl_strcmp(query->recalc.why, "off-route") == 0)
            m_calroutetype = RCT_OffRoute;
        if(nsl_strcmp(query->recalc.why, "detour") == 0)
            m_calroutetype = RCT_Detour;
    }
}
/* See description in header file. */
void OnboardRouteParameters::SetupViaPoints(data_nav_query*  query,
                                                data_util_state* /*pds*/)
{

    for(int i=0;i<CSL_VectorGetLength(query->via_point);i++)
    {
        data_via_point* ptr = (data_via_point*)CSL_VectorGetPointer(query->via_point,i);
        Ncdb::WorldPoint obj;
        obj.x = ptr->point.lon;
        obj.y = ptr->point.lat;
        m_viaPoints.push_back(obj);
    }

}

/* See description in header file. */
RouteRequestResult::RouteRequestResult(Ncdb::ReturnCode         calcResult,
                                       OnboardRouteParametersPtr parameters)
    : m_calcResult(calcResult),
      m_parameters(parameters)
{
    m_box = DefaultBoundBox;
}

/* See description in header file. */
RouteRequestResult::~RouteRequestResult()
{
}

// Local functions...
// Following functions do not need to access any private member fields of classes.
static NB_NavigateRouteError ConvertNcdbToNbErrors(Ncdb::ReturnCode err)
{
    NB_NavigateRouteError nbError = NB_NRE_None;
    switch (err)
    {
        case Ncdb::NCDB_OK:
        case Ncdb::NCDB_MORE_RECORDS:
        {
            nbError    = NB_NRE_None;
            break;
        }
        case Ncdb::NCDB_OPEN_ERR:
        case Ncdb::NCDB_READ_ERR:
        case Ncdb::NCDB_WRITE_ERR:
        case Ncdb::NCDB_ACCESS_DENIED:
        case Ncdb::NCDB_INVALID_FORMAT:
        case Ncdb::NCDB_INVALID_SUFFIX:
        case Ncdb::NCDB_NOT_FOUND:
        case Ncdb::NCDB_EMPTY_DATA:
        case Ncdb::NCDB_END_OF_QUERY:
        {
            nbError    = NB_NRE_CannotRoute;
            break;
        }
        case Ncdb::NCDB_UNSUPPORTED:
        {
            nbError    = NB_NRE_NoMatch;
            break;
        }
        case Ncdb::NCDB_ORIGIN_NOT_ACCESSIBLE:
        {
            nbError    = NB_NRE_BadOrigin;
            break;
        }
        case Ncdb::NCDB_DEST_NOT_ACCESSIBLE:
        {
            nbError = NB_NRE_BadDestination;
            break;
        }
        case Ncdb::NCDB_TOO_LONG:
        {
            nbError    = NB_NRE_PedRouteTooLong;
            break;
        }
        case Ncdb::NCDB_ABORT:
        {
            nbError = NB_NRE_ServerError;
            break;
        }
        case Ncdb::NCDB_TIME_OUT:
        {
            nbError = NB_NRE_TimedOut;
            break;
        }
        case Ncdb::NCDB_NO_ALTERNATE:
        {
            nbError    = NB_NRE_NoDetour;
            break;
        }
        case Ncdb::NCDB_FAIL:
        case Ncdb::NCDB_INVALID:
        case Ncdb::NCDB_NO_MEMORY:
        default:
            nbError = NB_NRE_UnknownError;;
            break;
    }
    return nbError;
}

static NB_NavigateRouteError ConvertNbErrorsToRouteError(NB_Error err)
{
    switch (err)
    {
        case NE_OK:
        {
            return NB_NRE_None;
        }
        case NEROUTE_CANNOTROUTE:
        {
            return NB_NRE_CannotRoute;
        }

        case NEROUTE_NONE:
        {
            return NB_NRE_EmptyRoute;
        }
        case NEROUTE_NOMATCH:
        {
            return NB_NRE_NoMatch;
        }
        case NEROUTE_BADORG:
        {
            return NB_NRE_BadOrigin;
        }
        case NEROUTE_BADDEST:
        {
            return NB_NRE_BadDestination;
        }
        case NEROUTE_PEDROUTETOOLONG:
        {
            return NB_NRE_RouteTooLong;
        }
        case NE_CANCELLED:
        {
            //@todo: mapping to correct error?
            return NB_NRE_None;
        }
        case NEROUTE_TIMEOUT:
        {
            return NB_NRE_TimedOut;
        }
        case NEROUTE_NODETOUR:
        {
            return NB_NRE_NoDetour;
        }
        default:
            return NB_NRE_UnknownError;
    }
    return NB_NRE_UnknownError;
}

typedef const Ncdb::WorldPoint&       RPoint;

static NB_Error ConvertPoint(data_util_state* pds,
                             data_point* pt,
                             RPoint point)
{
    NB_Error err = NE_OK;
    err = data_point_init(pds, pt);
    pt->lat = point.y;
    pt->lon = point.x;
    return err;
}

static NB_Error ConvertPoint(data_util_state* pds,
                             data_point* pt,
                             double lat,
                             double lon)
{
    NB_Error err = NE_OK;
    DATA_REINIT(pds, err, pt, data_point);
    pt->lat = lat;
    pt->lon = lon;
    return err;
}


static inline void ConvertString(data_util_state*       pds,
                                 data_string*           prs,
                                 const Ncdb::UtfString& str)
{
    data_string_set(pds, prs, str.GetBuffer());
}

static inline void ConvertString(data_util_state* pds,
                                 data_string*     prs,
                                 const char*      str)
{
    data_string_set(pds, prs, str);
}

static inline NB_Error ConvertBox(data_util_state*       pds,
                                  data_box*              prb,
                                  const Ncdb::WorldRect& box)
{
    NB_Error err = NE_OK;
    err = ConvertPoint(pds, &prb->topLeft, box.m_Left, box.m_Top);
    err = err ? err : ConvertPoint(pds, &prb->bottomRight, box.m_Right, box.m_Bottom);
    return err;
}


static inline void ConvertBlob(data_util_state*       pds,
                               data_blob*             pb,
                               const Ncdb::UtfString& str)
{
    if (str.Length() > 0)
    {
        NB_Error err = NE_OK;
        DATA_REINIT(pds, err, pb, data_blob);
        data_blob_set(pds, pb, (byte*)str.GetBuffer(), (size_t)str.Length()+1);
    }
}

static inline void ConvertBlob(data_util_state*       pds,
                               data_blob*             pb,
                               const string& str)
{
    if (!str.empty())
    {
        NB_Error err = NE_OK;
        DATA_REINIT(pds, err, pb, data_blob);
        data_blob_set(pds, pb, (byte*)str.data(), (size_t)str.size()+1);
    }
}

// Current oboard route does not support this..
static NB_Error ConvertTrafficRegion(data_util_state*   /*pds*/,
                                     struct CSL_Vector* /*vec_traffic_regions*/,
                                     void*              /*data*/)
{
    return NE_OK;
}

static NB_Error ConvertCrossStreets(data_util_state*           pds,
                                    CSL_Vector*                crossStreets,
                                    const Ncdb::CrossroadList& roadList)
{

    NB_Error err = NE_OK;
    data_cross_street cs, *pcs = &cs;

    const Ncdb::Crossroad* pr = roadList.begin();
    while (pr != roadList.end()) {
        DATA_INIT(pds, err, pcs, data_cross_street);

        const Ncdb::UtfString& leftName  = pr->getLeftMostCrossStreetName();
        const Ncdb::UtfString& rightName = pr->getRightMostCrossStreetName();
        Ncdb::UtfString mergedName("");
        if (!leftName.IsEmpty() && !rightName.IsEmpty())
        {
            mergedName  = leftName;
            mergedName += "/";
            mergedName += rightName;
        }
        else if (!leftName.IsEmpty() || !rightName.IsEmpty())
        {
            mergedName = leftName.IsEmpty() ? leftName : rightName;
        }

        ConvertString(pds, &pcs->name, mergedName);

        APPEND_DATA_POINTER(crossStreets, pcs, err);
        if (err)
        {
            DATA_FREE(pds, pcs, data_cross_street);
            return err;
        }
        ++pr;
    }

    return err;
}

static NB_Error ConvertRealisticSign(data_util_state*     pds,
                                     CSL_Vector*          realisticSigns,
                                     const Ncdb::SARInfo& sarInfo)
{
    NB_Error err = NE_OK;
    data_realistic_sign sign, *ps = &sign;

    DATA_INIT(pds, err, ps, data_realistic_sign);
    ConvertString(pds, &ps->id, sarInfo.SARID);
    ConvertString(pds, &ps->portrait_id, sarInfo.PortraitSARID);
    ConvertString(pds, &ps->type, sarInfo.SARType);
    ConvertString(pds, &ps->dataset_id, sarInfo.DataSetID);
    ConvertString(pds, &ps->version, sarInfo.Version);

    //@todo: check ps->position...

    APPEND_DATA_POINTER(realisticSigns, ps, err);
    return err;
}

// static NB_Error ConvertLaneInfo(pds, &pnm->lane_info, NULL);

static void ConvertRouteNumberInfo(data_util_state*        pds,
                                   data_route_number_info* pri,
                                   const Ncdb::RoadInfo&   roadInfo)
{
    ConvertString(pds, &pri->class_, roadInfo.getRouteType());
    ConvertString(pds, &pri->name, roadInfo.getRouteName());
    ConvertString(pds, &pri->dir, roadInfo.getRouteDir());
}

static void ConvertCountryInfo(data_util_state*         pds,
                               data_country_info*       pri,
                               const CountryInfo& countryInfo)
{
    ConvertString(pds, &pri->code,
                  nbcommon::StringUtility::NumberToString(countryInfo.GetId()).c_str());

    // ConvertString(pds, &pri->driving_side, countryInfo.GetDrivingSide());
}

static NB_Error ConvertExitNumber(data_util_state*        pds,
                                  data_exit_number*       prn,
                                  const Ncdb::ExitNumber& exitNumber)
{
    ConvertString(pds, &prn->number, exitNumber.GetExitNumber());
    ConvertBlob(pds, &prn->pronun,
                exitNumber.GetPronun().Length() > 0 ?
                    exitNumber.GetPronun() : exitNumber.GetExitNumber());
    return NE_OK;
}

static NB_Error ConvertRoadInfo(data_util_state*         pds,
                                data_roadinfo*           pri,
                                const Ncdb::RoadInfo&    roadInfo,
                                const CountryInfo& countryInfo,
                                const Ncdb::ExitNumber* exitNumber = NULL)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, pri, data_roadinfo);

    ConvertRouteNumberInfo(pds, &pri->route_number_info, roadInfo);
    ConvertCountryInfo(pds, &pri->country_info, countryInfo);
    if (exitNumber)
    {
        ConvertExitNumber(pds, &pri->exit_number, *exitNumber);
    }

    // attributes..
    const Ncdb::UtfString& str = roadInfo.getPrimaryName();
    ConvertString(pds, &pri->primary, str);
    ConvertString(pds, &pri->secondary, roadInfo.getSecondaryName());
    ConvertBlob(pds, &pri->pronun, roadInfo.getPronunKey().Length() > 0 ? \
                roadInfo.getPronunKey() : roadInfo.getPrimaryName());

    pri->unnamed = !(str.Length() > 0);

    // @todo: remove hard-coded values...
    pri->unpaved = FALSE;
    pri->toward  = FALSE;
    pri->trans   = 0;

    return err;
}

static double
ConvertGuidancePoints(data_util_state* pds,
                      const Ncdb::GuidancePointList&  guidancePtlist,
                      data_guidance_point* pnr)
{
    if (guidancePtlist.empty())
    {
        return 0;
    }

    NB_Error err = NE_OK;
    DATA_REINIT(pds, err, pnr, data_guidance_point);

    int listsize = guidancePtlist.size();
    double maxturndis = 0.0;
    int guidanceCount = 0;
    for(int i = 0; i < listsize; i++)
    {
        const Ncdb::GuidancePoint& guidancePoint =  guidancePtlist[i];
        if(guidanceCount >= GUIDANCE_POINTS_MAX)
        {
            break;
        }

        ConvertPoint(pds, &pnr->point, guidancePoint.getPoint());
        pnr->maneuver_point_offset = guidancePoint.getOffset();
        pnr->use_for_prepare = guidancePoint.IsPrepare();
        pnr->use_for_turn = guidancePoint.IsTurn();
        Ncdb::NGType ntype = guidancePoint.GetType();
        pnr->gptype = ntype == Ncdb::NG_INFO_NONE ? 0 : ntype;
        ConvertString(pds, &pnr->description, guidancePoint.GetDesc());
        ConvertBlob(pds,&pnr->gpprep_pronun, guidancePoint.getPreposition());
        ConvertBlob(pds,&pnr->gp_pronun, guidancePoint.getPronunKey());

        maxturndis = MAX(maxturndis,guidancePoint.GetMaxTurnDistance());
        guidanceCount++;
        pnr++;
    }

    return maxturndis;
}

class LaneGuidanceItemCollector
{
public:
    LaneGuidanceItemCollector(data_util_state* pds,  CSL_Vector* items)
        : m_pds(pds),
          m_items(items)
    {}

    ~LaneGuidanceItemCollector() {}

    void operator() (const Ncdb::MnvrLaneInfo& laneItem)
    {
        data_lane_guidance_item item;
        NB_Error err = NE_OK;
        DATA_INIT(m_pds, err,&item, data_lane_guidance_item);

        //@todo: Check if this is right.
        if (laneItem.GetLaneType())
        {
            ConvertLaneItemTemplate(&item.lane_item, laneItem.GetLaneType());
        }
        else if (laneItem.GetDividerType())
        {
            ConvertLaneItemTemplate(&item.divider_item, laneItem.GetDividerType());
        }
        else if (laneItem.IsHighlight())
        {
            // @todo: hard-coded value copied from NCDB, check if it is correct.
            ConvertLaneItemTemplate(&item.highlight_arrow_item,
                                    laneItem.GetArrowType().get(1));

        }
        else {
            ConvertLaneItemTemplate(&item.no_highlight_arrow_item,
                                    GetArrowType(laneItem));

        }

        APPEND_DATA_POINTER(m_items, &item, err);
        if (err)
        {
            data_lane_guidance_item_free(m_pds, &item);
        }
    }

private:
    NB_Error ConvertLaneItemTemplate(data_lane_item* pli, uint32 item)
    {
        NB_Error err = NE_OK;
        DATA_REINIT(m_pds, err, pli, data_lane_item);
        pli->item = item;
        return err;
    }

    uint32 GetArrowType(const Ncdb::MnvrLaneInfo &laneInfo)
    {
        uint32 ret = 0;
        if (laneInfo.Straight())
        {
            ret |= AT_STRAIGHT;
        }
        if (laneInfo.SlightRight())
        {
            ret |= AT_SLIGHT_RIGHT;
        }
        if (laneInfo.Right())
        {
            ret |= AT_RIGHT;
        }
        if (laneInfo.HardRight())
        {
            ret |= AT_HARD_RIGHT;
        }
        if (laneInfo.UTurnLeft())
        {
            ret |= AT_UTURN_LEFT;
        }
        if (laneInfo.HardLeft())
        {
            ret |= AT_HARD_LEFT;
        }
        if (laneInfo.Left())
        {
            ret |= AT_LEFT;
        }
        if (laneInfo.SlightLeft())
        {
            ret |= AT_SLIGHT_LEFT;
        }
        if (laneInfo.MergeRight())
        {
            ret |= AT_MERGE_RIGHT;
        }
        if (laneInfo.MergeLeft())
        {
            ret |= AT_MERGE_LEFT;
        }
        if (laneInfo.MergeLanes())
        {
            ret |= AT_MERGE_LANES;
        }
        if (laneInfo.UTurnRight())
        {
            ret |= AT_UTURN_RIGHT;
        }
        if (laneInfo.SecondRight())
        {
            ret |= AT_SECOND_RIGHT;
        }
        if (laneInfo.SecondLeft())
        {
            ret |= AT_SECOND_LEFT;
        }
        if (0 == ret)
        {
            ret |= AT_STRAIGHT;
        }
        return ret;
    }

    data_util_state* m_pds;
    CSL_Vector*      m_items;
};

static NB_Error ConvertLaneInfo(data_util_state*        pds,
                                data_lane_info*         pli,
                                const Ncdb::LaneGuidanceInfo& laneInfo)
{
    NB_Error err = NE_OK;
    DATA_REINIT(pds, err, pli, data_lane_info);
    CHECK_ERROR(err);

    pli->number_of_lanes = laneInfo.GetNumLanesUsedInManeuver();
    pli->lane_position   = laneInfo.GetStartPosition();
    ConvertBlob(pds, &pli->lg_pronun, laneInfo.GetPronun());
    ConvertBlob(pds, &pli->lgprep_pronun, laneInfo.GetPrepPronun());

    const Ncdb::MnvrLaneInfoList& itemList = laneInfo.GetLaneInfoList();

    for_each(itemList.begin(), itemList.end(),
             LaneGuidanceItemCollector(pds, pli->vec_lane_guidance_items));

    return err;
}

static NB_Error ConvertDataPolyline(data_util_state*            pds,
                                    data_polyline*              ppl,
                                    const Ncdb::WorldPointList& points)
{
    NB_Error err = NE_OK;
    DATA_REINIT(pds, err, ppl, data_polyline);

    ppl->numsegments = points.size();
    ppl->segments = NB_ALLOC(pl_segment, ppl->numsegments);
    NB_BZERO_P(ppl->segments);

    pl_segment* ps = ppl->segments;
    double len = 0.0;
    for (uint32 i = 0; i < ppl->numsegments; i++) {
        ps->lat = points[i].y * POLYLINE_LL_SCALE_FACTOR;
        ps->lon = points[i].x * POLYLINE_LL_SCALE_FACTOR;
        ps->heading = POLYLINE_INVALID_HEADING;
        ps->len = -1;
        if (i != ppl->numsegments - 1)
        {
            len = NB_SpatialGetLineOfSightDistance(
                points[i].y, points[i].x,
                points[i+1].y, points[i+1].x,
                &ps->heading);
             ps->len = (int32) (POLYLINE_LEN_SCALE_FACTOR * len);
             ppl->length += len;
        }

        SET_MIN(ppl->min_lat, points[i].y);
        SET_MAX(ppl->max_lat, points[i].y);
        SET_MIN(ppl->min_lon, points[i].x);
        SET_MAX(ppl->max_lon, points[i].x);
        ++ps;
    }

    return err;
}

static NB_Error ConvertCrossRoad(data_util_state* pds,
                                 data_cross_street* pcr,
                                 const Ncdb::FirstManeuverCrossroad* crossroad)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, pcr, data_cross_street);
    ConvertString(pds, &pcr->name, crossroad->getCrossStreetName());
    ConvertString(pds, &pcr->pronun, crossroad->getPronunKey().Length() > 0 ? \
                  crossroad->getPronunKey() : crossroad->getCrossStreetName());
    return err;
}

static NB_Error
ConvertManeuevers(data_util_state*   pds,
                  struct CSL_Vector* maneuvers,
                  const Ncdb::AutoSharedPtr<Ncdb::ManeuverList>& manlist)
{
    NB_Error err = NE_OK;
    int maneuversize = manlist ? manlist->size() : 0;
    if (!maneuversize) //@todo: revisit this.
    {
        return err;
    }

    data_nav_maneuver maneuver, *pnm = &maneuver;

    for(int i = 0; i < maneuversize; i++)
    {
        err = data_nav_maneuver_init(pds, pnm);
        CHECK_ERROR(err);
        Ncdb::Maneuver man   = manlist->get(i);
        pnm->max_instruction_distance = man.getMaxDistance();
        pnm->distance        = man.getLength();
        pnm->current_heading = man.getHeading();
        pnm->stack_advise    = man.isStacked();
        data_string_set(pds, &pnm->command, GetManAbbrevByType(man.getTurnType()));
        pnm->speed = man.getEstimatedTime() <= 0 ? 0 : pnm->distance/(man.getEstimatedTime()*3600);
        // pnm->disable_guidance = man. //@todo:

        err = ConvertPoint(pds, &pnm->point, man.getPoint());
        CHECK_ERROR(err);

        const CountryInfo&      countryInfo = man.GetCurCountry();
        const Ncdb::ExitNumber& exitNumber  = man.GetMvrExitNumber();

        err = ConvertRoadInfo(pds, &pnm->current_roadinfo, man.getCurRoadInfo(),
                              countryInfo, &exitNumber);
        CHECK_ERROR(err);
        err = ConvertRoadInfo(pds, &pnm->turn_roadinfo, man.getTurnRoadInfo(),
                              countryInfo, &exitNumber);
        CHECK_ERROR(err);
        ConvertTrafficRegion(pds, pnm->vec_traffic_regions, NULL);
        ConvertLaneInfo(pds, &pnm->lane_info, man.GetMvrLaneGuidance());

        // ConvertRoadInfo(pds, &pnm->intersection_roadinfo, XXX:); //@todo:

        err = ConvertDataPolyline(pds, &pnm->polyline, man.getPolyline());
        CHECK_ERROR(err);

        err = ConvertCrossStreets(pds, pnm->vec_cross_streets, man.getCrossroads());
        CHECK_ERROR(err);

        ConvertRealisticSign(pds, pnm->vec_realistic_signs, man.GetSARInfo());
        CHECK_ERROR(err);

        // max turn distance
        double maxturndis = ConvertGuidancePoints(pds,
                                                  man.GetGuidancePointList(),
                                                  pnm->guidance_pointer);
        pnm->max_turn_distance_for_street_count.max_turn_distance = maxturndis;

        Ncdb::FirstManeuverCrossroadList crossroads =
                man.getFirstManeuverCrossroadList();
        if (!crossroads.empty())
        {
            const Ncdb::FirstManeuverCrossroad* iter = crossroads.begin();
            const Ncdb::FirstManeuverCrossroad* end = crossroads.end();
            data_cross_street crossroad, *pcr = &crossroad;
            while (iter != end) {
                err = ConvertCrossRoad(pds, pcr, iter);
                if (err == NE_OK)
                {
                    APPEND_DATA_POINTER(pnm->vec_cross_streets, pcr, err);
                }
                else
                {
                    DATA_FREE(pds, pcr, data_cross_street);
                }
                ++iter;
            }
        }

        APPEND_DATA_POINTER(maneuvers, pnm, err);
    }

    return err;
}

static NB_Error ConvertLabelPoint(data_util_state*  pds,
                                  data_label_point* pnp,
                                  RPoint            labelPt)
{
    return ConvertPoint(pds, &pnp->point, labelPt);
}

static NB_Error ConvertSpecialSpeedZone(data_util_state*  pds,
                                        data_special_speed_zone* psz,
                                        const Ncdb::SpeedType& speedType)
{
    NB_Error err = NE_OK;
    Ncdb::SPECIAL_SPEED_ZONE_TYPE zoneType = speedType.GetSpeedZoneType();
    switch (zoneType)
    {
        case Ncdb::zone_school:
        {
            ConvertString(pds, &psz->speed_zone_type, "SZ");
            break;
        }
        default:
        {
            ConvertString(pds, &psz->speed_zone_type, "");
            break;
        }
    }

    psz->warn_ahead = speedType.GetWarnAhead();
    return err;
}

static NB_Error ConvertSpeedRegion(data_util_state*         pds,
                                   data_speed_region*       psr,
                                   const Ncdb::SpeedRegion& region)
{
    NB_Error err = NE_OK;

    psr->start_maneuver_index  = region.GetStartManeuverIndex();
    psr->end_maneuver_index    = region.GetEndManeuverIndex();
    psr->start_maneuver_offset = region.GetStartManeuverOffset();
    psr->start_maneuver_offset = region.GetStartManeuverOffset();
    psr->speed_limit = region.GetSpeedLimitedList()->get(0).GetSpeedLimitedValue();

    err = ConvertSpecialSpeedZone(pds, psr->special_speed_zone,
                                  region.GetSpeedType());

    // process version,signId...
    Ncdb::UtfString streetname = region.GetCountryCode();
    streetname += "-SPEED";
    ConvertString(pds, &psr->dataset_id, streetname);

    int limit = 0;
    // @todo: copied from NCDB library, check if they are correct.
    if ((-1 != streetname.Find("USA-SPEED",0)) ||
        (-1 != streetname.Find("GBR-SPEED",0)))
    {
        limit = floor(psr->speed_limit * MPS_MPH + 0.5);
        if (-1 != streetname.Find("USA-SPEED",0))
        {
            //@todo: remove hard-coded version...
            ConvertString(pds, &psr->version, "130405");
            ConvertString(pds, &psr->version_highlighted, "130405");
        }
    }
    else
    {
        limit = floor(psr->speed_limit * MPS_KPH + 0.5);
    }

    string sign = nbcommon::StringUtility::NumberToString(limit);

    sign += data_string_get(pds, &psr->special_speed_zone->speed_zone_type);
    ConvertString(pds, &psr->sign_id, sign.c_str());

    sign += "-H";
    ConvertString(pds, &psr->sign_id_highlighted, sign.c_str());

    return err;
}

typedef Ncdb::AutoSharedPtr<Ncdb::SpeedRegionList> SpeedRegionListPtr;
static NB_Error ConvertSpeedRegions(data_util_state*          pds,
                                    data_speed_limits_data*   psr,
                                    const SpeedRegionListPtr& regionList)
{
    NB_Error err = NE_OK;
    DATA_REINIT(pds, err, psr, data_speed_limits_data);
    const Ncdb::SpeedRegion* iter  = regionList->begin();
    const Ncdb::SpeedRegion* end = regionList->end();
    data_speed_region region, *pr = &region;
    while (iter != end) {
        DATA_INIT(pds, err, pr, data_speed_region);
        err = ConvertSpeedRegion(pds, pr, *iter);
        if (!err)
        {
            APPEND_DATA_POINTER(psr->speed_regions, pr, err);
        }
        if (err)
        {
            DATA_FREE(pds, pr, data_speed_region);
            return err;
        }

        ++iter;
    }

    return err;
}

static NB_Error ConvertSpecialRegion(data_util_state*           pds,
                                     data_special_region*       psr,
                                     const Ncdb::SpecialRegion* /*region*/)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, psr, data_special_region);

    // Almost same as ConvertSpeedRegion
    //@todo: implement this....

    return err;
}

// we can't use const referece of route here, because some member functions in route
// is not accessible for const references.
static NB_Error ConvertNavRoute(data_util_state*       pds,
                                data_nav_route*        pnr,
                                Ncdb::Route&           route,
                                const std::string&     routeID,
                                const Ncdb::WorldRect& box,
                                NB_RouteParameters*    params)
{
    NB_Error err = NE_OK;
    DATA_INIT(pds, err, pnr, data_nav_route);
    err = ConvertManeuevers(pds, pnr->vec_nav_maneuver, route.GetManeuverList());
    CHECK_ERROR(err);

    // err = ConvertDetourAvoids(pds, pnr->vec_detour_avoid, );
    // err = ConvertCameras();

    pnr->ferry_on_route = route.IsFerryInRoute() ? TRUE : FALSE;
    pnr->hasRouteExtents = FALSE; // @todo: check this.

#define LAZY_DATA_REINIT(P, T)                  \
    DATA_REINIT(pds, err, &pnr->P, T)
#define LAZIER_DATA_REINIT(X)                   \
    DATA_REINIT(pds, err, &pnr->X, data_##X)

    LAZIER_DATA_REINIT(extended_content);

    // Onboard version does not support traffic incidents...
    LAZIER_DATA_REINIT(traffic_incidents);
    LAZIER_DATA_REINIT(traffic_record_identifier);

    LAZY_DATA_REINIT(standard_traffic_flow, data_traffic_flow);
    LAZY_DATA_REINIT(historical_traffic_flow, data_traffic_flow);
    LAZY_DATA_REINIT(realtime_traffic_flow, data_traffic_flow);

    ConvertLabelPoint(pds, &pnr->label_point, route.GetLabelPoint());

    ConvertBox(pds, &pnr->route_extents, box);

    err = ConvertSpeedRegions(pds, pnr->speed_regions, route.GetSpeedRegionList());
    CHECK_ERROR(err);

    // First Major Road
    Ncdb::RoadInfo majorRoad;
    CountryInfo countryInfo; // lack of Ncdb namespace...
    if (route.GetFirstMajorRoad(majorRoad, countryInfo) == Ncdb::NCDB_OK)
    {
        err = ConvertRoadInfo(pds, pnr->first_major_road, majorRoad, countryInfo);
        CHECK_ERROR(err);
    }

    err = data_route_style_copy(pds, &pnr->route_style,
                                &params->dataQuery.route_style);
    CHECK_ERROR(err);

    pnr->tolls_on_route = TO_NB_BOOL(route.IsTollRoadInRoute());
    pnr->hov_lanes_on_route = TO_NB_BOOL(route.IsHOVInRoute());
    pnr->highway_on_route = TO_NB_BOOL(route.IsHwyInRoute());
    pnr->unpaved_on_route = TO_NB_BOOL(route.IsUnpavedInRoute());

    Ncdb::AutoSharedPtr<Ncdb::SpecialRegionList> specialRegions =
            route.GetSpecialRegionList();
    if (!specialRegions->empty())
    {
        const Ncdb::SpecialRegion* iter = specialRegions->begin();
        const Ncdb::SpecialRegion* end = specialRegions->end();
        data_special_region special_region, *psr = &special_region;
        while (iter != end) {
            DATA_INIT(pds, err, psr, data_special_region);
            err = ConvertSpecialRegion(pds, psr, iter);
            if (err == NE_OK)
            {
                APPEND_DATA_POINTER(pnr->vec_special_regions, psr, err);
            }

            if (err)
            {
                DATA_FREE(pds, psr, data_special_region);
                return err;
            }

            ++iter;
        }
    }

    ConvertBlob(pds, &pnr->route_id, routeID);
    pnr->navManeuverTotal  = route.GetManeuverList()->size();
    pnr->navManeuverStart  = 0;
    pnr->navManeuverEnd    = pnr->navManeuverTotal - 1;
    pnr->polyline_complete = TRUE;
    pnr->polyline_length = route.GetRouteLength();

#undef LAZIER_DATA_REINIT
#undef LAZY_DATA_REINIT

    return err;
}

static NB_Error
CreatePronunNode(data_util_state* pds, data_nav_reply_mult_form* pnr,
                 data_string* key, data_string* value)
{
    NB_Error err  = NE_OK;
    data_pronun_extended prounItem,*ppe = &prounItem;
    DATA_INIT(pds, err, ppe, data_pronun_extended);
    CHECK_ERROR(err);
    data_string_copy(pds, &ppe->key, key);
    //@TODO:We should get the pronun data and the full text of the road from ncdb by the key.
    data_string_copy(pds, &ppe->text, value);
    APPEND_DATA_POINTER(pnr->pronun_list.vec_pronun_extended, ppe, err);
    if (err)
    {
        data_pronun_extended_free(pds, ppe);
    }
    return err;
}

static NB_Error
CreatePronunList(data_util_state* pds, data_nav_reply_mult_form* pnr)
{
    NB_Error err  = NE_OK;

    for (int i = 0; i < CSL_VectorGetLength(pnr->vec_routes); ++i)
    {
        data_nav_route* nav_route = (data_nav_route*)CSL_VectorGetPointer(pnr->vec_routes, i);
        for (int j = 0; j < CSL_VectorGetLength(nav_route->vec_nav_maneuver); ++j)
        {
            data_nav_maneuver* nav_man = (data_nav_maneuver*)CSL_VectorGetPointer(nav_route->vec_nav_maneuver, j);

            err = CreatePronunNode(pds, pnr,
                                   (data_string*)&nav_man->current_roadinfo.pronun.data,
                                   &nav_man->current_roadinfo.primary);
            err = err ? err : CreatePronunNode(pds, pnr,
                                               (data_string*)&nav_man->current_roadinfo.exit_number.pronun.data,
                                               &nav_man->current_roadinfo.exit_number.number);
            err = err ? err : CreatePronunNode(pds, pnr,
                                               (data_string*)&nav_man->turn_roadinfo.pronun.data,
                                               &nav_man->turn_roadinfo.primary);
            err = err ? err : CreatePronunNode(pds, pnr,
                                               (data_string*)&nav_man->turn_roadinfo.exit_number.pronun.data,
                                               &nav_man->turn_roadinfo.exit_number.number);
            for (int k = 0; k < CSL_VectorGetLength(nav_man->vec_cross_streets); ++k)
            {
                data_cross_street* crossStreet = (data_cross_street*)CSL_VectorGetPointer(nav_man->vec_cross_streets, k);

                err = err ? err : CreatePronunNode(pds, pnr,
                                               (data_string*)&crossStreet->pronun,
                                               &crossStreet->name);
            }
        }
    }
    return err;
}

static NB_Error
ConvertNavReplyMultiForm(data_util_state*              pds,
                         Ncdb::AutoArray<Ncdb::Route>& routes,
                         const std::vector<std::string>& routeIDs,
                         const Ncdb::WorldRect&        box,
                         NB_RouteParameters*           params,
                         data_nav_reply_mult_form*     pnr)
{
    NB_Error err  = NE_OK;

    data_nav_route    nav_route;

    DATA_REINIT(pds, err, pnr, data_nav_reply_mult_form);
    CHECK_ERROR(err);

    // size have been checked before...
    int totalRoutes = routes.size();
    const Ncdb::Route& route = routes[0];

    // origin and destination
    Ncdb::WorldPoint originPt, destPt;
    originPt.Clear();
    destPt.Clear();
    route.GetOriginConnection().getPointLocation(originPt);
    route.GetDestinationConnection().getPointLocation(destPt);

    err = ConvertPoint(pds, &pnr->origin_location.point, originPt);
    err = err ? err : ConvertPoint(pds,
                                   &pnr->destination_location.point,
                                   destPt);
    CHECK_ERROR(err);

    // lable point
    err = ConvertLabelPoint(pds, &pnr->current_label_point, route.GetLabelPoint());
    CHECK_ERROR(err);

    for (int i = 0; i < totalRoutes; i++) {
        err = ConvertNavRoute(pds, &nav_route, routes[i], routeIDs[i], box, params);
        if (err == NE_OK)
        {
            APPEND_DATA_POINTER(pnr->vec_routes, &nav_route, err);   
        }
        else
        {
            data_nav_route_free(pds, &nav_route);
        }
     }
    CHECK_ERROR(err);
    CreatePronunList(pds, pnr);
    return err;
}

// local function, caller should make sure params are valid..
static NB_Error ConvertNavReply(NB_Context*                   context,
                                NB_RouteParameters*           params,
                                Ncdb::AutoArray<Ncdb::Route>& routes,
                                const std::vector<std::string>& routeIDs,
                                const Ncdb::WorldRect&        box,
                                data_nav_reply*               pnr)
{
#undef check_error
#define check_error()                           \
    if (err != NE_OK)                           \
    {                                           \
        data_nav_reply_free(pds, pnr);          \
        return err;                             \
    }

    data_util_state* pds = NB_ContextGetDataState(context);
    if (!pds)
    {
        return NE_INVAL;
    }

    data_nav_reply_free(pds, pnr);
    NB_Error err = NE_OK;
    err = data_nav_reply_init(pds, pnr);
    check_error();

    data_nav_reply_mult_form mult_reply;
    err = data_nav_reply_mult_form_init(pds, &mult_reply);
    check_error();

    err = ConvertNavReplyMultiForm(pds, routes, routeIDs, box, params, &mult_reply);

    err = err ? err : data_nav_reply_from_mult_form(pds, &mult_reply, pnr, 0);
    err = err ? err : data_nav_reply_mult_form_copy(pds, &pnr->route_selector_reply, &mult_reply);
    pnr->active_route = 0;
    data_nav_reply_mult_form_free(pds, &mult_reply);

    if (err != NE_OK)
    {
        data_nav_reply_free(pds, pnr);
    }

    return err;
}

static NB_Error ConvertRouteSummary(NB_Context* context, Ncdb::FastETA& eta, CSL_Vector* routeSummarys)
{
    data_util_state* pds = NB_ContextGetDataState(context);
    if (!pds)
    {
        return NE_INVAL;
    }
    data_route_summary data;
    data_route_summary_init(pds, &data);
    data.travel_time = eta.m_eta;
    data.distance = eta.m_len;
    CSL_VectorAppend(routeSummarys, &data);
    return NE_OK;
}

/*! @} */

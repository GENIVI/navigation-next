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
   @file        OptionalLayersAgentImpl.cpp
   @defgroup    nbmap

   Description: Implementation of OptionalLayersAgentImpl

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#include "OptionalLayersAgentImpl.h"
#include "RouteManagerImpl.h"
#include "TrafficManagerImpl.h"
#include "GeographyManagerImpl.h"
#include "LocalLayerProvider.h"
#include "StringUtility.h"
#include "LayerAgentImpl.h"
#include "LayerFunctors.h"
#include <iterator>
#include "nbmacros.h"
#include "StringUtility.h"
#include <functional>
#include <algorithm>
#include "GeographicPolylineLayer.h"
#include "Lock.h"
#include "palevent.h"

using namespace nb;
using namespace nbmap;

/*! Function to find out layer agent with specified name. */
class NameEqualFunctor
{
public:
    NameEqualFunctor(shared_ptr<string> name)
            : m_pName(name) {}
    virtual ~NameEqualFunctor() {}

    bool operator()(const LayerAgentPtr& layer) const
    {
        bool result = false;
        if (layer)
        {
            result = nbcommon::StringUtility::IsStringEqual(m_pName, layer->GetName());
        }
        return result;
    }
private:
    shared_ptr<string> m_pName;
};

class EventQueue_LayerOperationParameter
{
public:
    typedef enum _LayerOperationType
    {
        LOT_Add = 0,
        LOT_Remove,
        LOT_Inval
    } LayerOperationType;

    EventQueue_LayerOperationParameter(
        OptionalLayersAgentImpl& agent,
        const vector<UnifiedLayerPtr>& layers,
        EventQueue_LayerOperationParameter::LayerOperationType type,
        shared_ptr<bool> isValid)
            : m_agent(agent),
              m_layers(layers),
              m_type(type),
              m_isValid(isValid){}
    virtual ~EventQueue_LayerOperationParameter(){}

    OptionalLayersAgentImpl& m_agent;
    vector<UnifiedLayerPtr>  m_layers;
    LayerOperationType       m_type;
    shared_ptr<bool>         m_isValid;
private:
    NB_DISABLE_COPY(EventQueue_LayerOperationParameter);
};

class UnifiedLayerCollector
{
public:
    UnifiedLayerCollector (vector<LayerPtr>* layers,
                           LocalLayerProvider* provider,
                           LayerOperator functor)
            : m_layers(layers),
              m_layerProvider(provider),
              m_singleLayerOperator(functor)
    {}
    virtual ~UnifiedLayerCollector(){}

    void operator() (const UnifiedLayerPtr& uLayer)
    {
        vector<UnifiedLayerPtr> tmpLayers = uLayer->GetAllChildrenLayers();
        tmpLayers.push_back(uLayer);
        vector<UnifiedLayerPtr>::const_iterator iter = tmpLayers.begin();
        vector<UnifiedLayerPtr>::const_iterator end  = tmpLayers.end();
        for (; iter != end; ++iter)
        {
            if (m_singleLayerOperator(m_layerProvider, *iter))
            {
                m_layers->push_back(*iter);
            }
        }
    }
private:
    vector<LayerPtr>*   m_layers;
    LocalLayerProvider* m_layerProvider;
    mem_fun1_t<bool, LocalLayerProvider, LayerPtr> m_singleLayerOperator;
};

class GeographicCollector
{
public:
    GeographicCollector(vector<Geographic*>* collector)
            : m_collector(collector){}
    ~GeographicCollector(){}
    void operator ()(const pair<Geographic*, GeographicLayerPtr>& p) const
    {
        if (p.first)
        {
            m_collector->push_back(p.first);
        }
    }
private:
    vector<Geographic*>* m_collector;
};

class DestroryTrafficParamter
{
public:
    DestroryTrafficParamter( void* pAgent, PAL_Event* pEvent) :
        pThis( pAgent ),
        pEvent( pEvent ){}

    void* pThis;
    PAL_Event* pEvent;
};

static const shared_ptr<string> emptyString;
static const UnifiedLayerPtr emptyLayer;


// Implementation of OptionalLayersAgentImpl
/* See description in header file. */
OptionalLayersAgentImpl::OptionalLayersAgentImpl(NB_Context* context,
                                                 shared_ptr<LayerManager> layerManager,
                                                 shared_ptr<LocalLayerProvider> layerProvider,
                                                 shared_ptr<LayerIdGenerator> layerIdGenerator)
        : UnifiedLayerListener(context),
          m_pLayerProvider(layerProvider),
          m_pLayerIdGenerator(layerIdGenerator),
          m_pLayerManager(layerManager),
          m_isValid(new bool(true))
{
    PAL_LockCreate(NB_ContextGetPal(context), &m_lock);
    shared_ptr<AggregateLayerProvider> aggregateLayerProvider =
            ContextBasedSingleton<AggregateLayerProvider>::getInstance(context);
    aggregateLayerProvider->RegisterUnifiedLayerListener(this, false);
}

void OptionalLayersAgentImpl::CCC_DestoryTrafficManager(PAL_Instance* /*pal*/, void* userData)
{
    DestroryTrafficParamter* paramter = static_cast<DestroryTrafficParamter *>( userData );
    OptionalLayersAgentImpl* pThis = static_cast<OptionalLayersAgentImpl *>( paramter->pThis );
    pThis->m_pTrafficManager.reset();

    PAL_EventSet(paramter->pEvent);
    delete paramter;
}

/* See description in header file. */
OptionalLayersAgentImpl::~OptionalLayersAgentImpl()
{
    if (m_isValid)
    {
        *m_isValid = false;
    }
    PAL_LockDestroy(m_lock);
    m_lock = NULL;

    PAL_Event* destroyingEvent = NULL;
    if (PAL_EventCreate(NB_ContextGetPal(m_pContext), &destroyingEvent) == PAL_Ok)
    {
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(NB_ContextGetPal(m_pContext),
                                      &OptionalLayersAgentImpl::CCC_DestoryTrafficManager,
                                      new DestroryTrafficParamter( this, destroyingEvent ),
                                      &taskId);
        PAL_EventWaitForEvent(destroyingEvent);
        PAL_EventDestroy(destroyingEvent);
        destroyingEvent = NULL;
    }
}

/* See description in header file. */
GeographyOperationPtr
OptionalLayersAgentImpl::AddGeographyManager(const string& layerId)
{
    return (m_pGeographyManager->AddGeographyLayerSuit(layerId));
}

void
OptionalLayersAgentImpl::RemoveGeographyManager(const string& layerId)
{
    m_pGeographyManager->RemoveGeographyLayerSuit(layerId);
}

shared_ptr <string> OptionalLayersAgentImpl::AddRouteLayer(const vector <RouteInfo*>& routeInfo)
{
    shared_ptr<string> routeId;
    if (m_pRouteManager)
    {
        routeId = m_pRouteManager->AddRouteLayer(routeInfo);
    }
    return routeId;
}

/* See description in header file. */
NB_Error OptionalLayersAgentImpl::RemoveRouteLayer(shared_ptr < string > routeLayerId)
{
    return  m_pRouteManager ? m_pRouteManager->RemoveRouteLayer(routeLayerId) : NE_NOTINIT;
}

/* See description in header file. */
NB_Error OptionalLayersAgentImpl::RemoveAllRouteLayers()
{
    NB_Error result = NE_NOTINIT;
    if (m_pRouteManager)
    {
        m_pRouteManager->RemoveAllRouteLayers();
        result = NE_OK;
    }
    return  result;
}

/* See description in header file. */
NB_Error OptionalLayersAgentImpl::EnableTraffic()
{
    NB_Error result = NE_NOTINIT;
    if (m_pTrafficManager)
    {
        m_pTrafficManager->EnableTraffic();
        result = NE_OK;
    }
    return  result;
}

/* See description in header file. */
NB_Error OptionalLayersAgentImpl::DisableTraffic()
{
    NB_Error result = NE_NOTINIT;
    if (m_pTrafficManager)
    {
        m_pTrafficManager->DisableTraffic();
        result = NE_OK;
    }
    return  result;
}

/* See description in header file. */
vector < NB_TrafficIncident > OptionalLayersAgentImpl::GetTrafficIncidents()
{
    return  m_pTrafficManager ? m_pTrafficManager->GetTrafficIncidents() :
            vector < NB_TrafficIncident >();
}


/* See description in header file. */
void OptionalLayersAgentImpl::RegisterTrafficManagerListener(TrafficManagerListener* listener)
{
    if (m_pTrafficManager)
    {
        m_pTrafficManager->RegisterListener(listener);
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::UnegisterTrafficManagerListener(TrafficManagerListener* listener)
{
    if (m_pTrafficManager)
    {
        m_pTrafficManager->UnregisterListener(listener);
    }
}

/* See description in header file. */
void
OptionalLayersAgentImpl::RegisterRouteManger(shared_ptr <RouteManager> routeManager)
{
    m_pRouteManager = routeManager;
}

/* See description in header file. */
void
OptionalLayersAgentImpl::RegisterTrafficManger(shared_ptr <TrafficManager> trafficManager)
{
    m_pTrafficManager = trafficManager;
}

void
OptionalLayersAgentImpl::RegisterGeographyManager(shared_ptr<GeographyManager> geographyManager)
{
    m_pGeographyManager = geographyManager;
}

/* See description in header file. */
const vector <LayerAgentPtr>&  OptionalLayersAgentImpl::GetOptionalLayerAgents()
{
    return m_pLayerAgents;
}

/* See description in header file. */
void OptionalLayersAgentImpl::UnifiedLayerUpdated()
{
    vector<LayerPtr> newOptionalLayers = m_pLayerProvider->GetOptionalLayerPtrVectorFromParent();

    if (newOptionalLayers.empty()) // No optional layers available anymore.
    {
        ClearOptionalLayers();
        return;
    }

    vector<LayerAgentPtr> tmpAgents;
    tmpAgents.swap(m_pLayerAgents);

    vector<LayerPtr> removedLayers;

    // 1. Create a layerAgent for optional layers that are newly added.
    vector<LayerPtr>::const_iterator iter = newOptionalLayers.begin();
    vector<LayerPtr>::const_iterator end  = newOptionalLayers.end();

    for (; iter != end; ++iter)
    {
        UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(iter->get());
        if (!uLayer)
        {
            continue;
        }

        //@todo: should compare Category instead of tile type.
        shared_ptr<string> type = uLayer->GetTileDataType();
        if (!type || !type->compare(TILE_TYPE_ROUTE) ||
            !(type->compare(TILE_TYPE_TRAFFIC)) || !(type->compare(TILE_TYPE_DVRT)))
        {
            // route and traffic layers are handled by RouteManager or TrafficManager.
            continue;
        }

        shared_ptr<string> name = uLayer->GetLayerName();
        if (!name || name->empty())
        {
            continue;
        }

        UnifiedLayerPtr newLayer = uLayer->Clone();
        if (!newLayer)
        {
            continue;
        }

        bool layerEnabled = newLayer->IsEnabled();

        LayerAgentPtr agentLayer;

        // If there is a LayerAgent of same name, replace it,
        // or create new LayerAgent for this cloned layer.
        vector<LayerAgentPtr> :: iterator result =
                find_if(tmpAgents.begin(), tmpAgents.end(), NameEqualFunctor(name));
        if (result != tmpAgents.end()) // replace existing one.
        {
            agentLayer = *result;
            LayerAgentImpl* agentLayerImpl = static_cast<LayerAgentImpl*>(agentLayer.get());
            if (agentLayerImpl)
            {
                LayerPtr oldLayer = agentLayerImpl->GetLayerPtr();
                if (m_pLayerProvider->RemoveLayer(oldLayer))
                {
                    removedLayers.push_back(oldLayer);
                }

                layerEnabled = agentLayerImpl->IsEnabled();
                agentLayerImpl->ResetLayer(newLayer);

                // Now move the found LayerAgent from tmpAgents into m_pLayerAgents.
                tmpAgents.erase(result);
            }
        }
        else // Completely new optional layer, create LayerAgent and store it.
        {
            agentLayer = LayerAgentImpl::CreateLayerAgent(m_pLayerManager,
                                                          newLayer,
                                                          m_pLayerProvider);
            if (!agentLayer)
            {
                continue;
            }
        }

        agentLayer->SetEnabled(layerEnabled);
        m_pLayerAgents.push_back(agentLayer);
    }

    // 2. All new layers are handled, if there are layers left in tmpAgents, discard them.
    vector<LayerAgentPtr>::const_iterator agentIter = tmpAgents.begin();
    vector<LayerAgentPtr>::const_iterator agentEnd  = tmpAgents.end();
    for (; agentIter != agentEnd; ++agentIter)
    {
        LayerAgentImpl* agentLayer = static_cast<LayerAgentImpl*> (agentIter->get());
        if (agentLayer)
        {
            LayerPtr clonedLayer = agentLayer->GetLayerPtr();
            if (m_pLayerProvider->RemoveLayer(clonedLayer))
            {
                removedLayers.push_back(clonedLayer);
            }

            agentLayer->ResetLayer(emptyLayer);
        }
    }

    // 4. Notify removed layers if necessary.
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (uLayerManager)
    {
        if (!removedLayers.empty())
        {
            uLayerManager->NotifyLayersRemoved(removedLayers);
        }
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::ClearOptionalLayers()
{
    if (m_pLayerAgents.empty())
    {
        return;
    }

    // Swap and clear layers in m_pOptionaLayers.
    vector<LayerAgentPtr> tmpAgents;
    tmpAgents.swap(m_pLayerAgents);

    vector<LayerPtr> removedLayers;
    vector<LayerAgentPtr>::iterator iter = tmpAgents.begin();
    vector<LayerAgentPtr>::iterator end  = tmpAgents.end();
    for (; iter != end; ++iter)
    {
        LayerAgentImpl* agentLayer = static_cast<LayerAgentImpl*>(iter->get());
        LayerPtr clonedLayer;
        if (agentLayer && (clonedLayer = agentLayer->GetLayerPtr()))
        {
            if (m_pLayerProvider->RemoveLayer(clonedLayer))
            {
                removedLayers.push_back(clonedLayer);
            }
            agentLayer->ResetLayer(emptyLayer);
        }
    }

    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (uLayerManager && !removedLayers.empty())
    {
            uLayerManager->NotifyLayersRemoved(removedLayers);
    }
}

namespace nbmap
{
class GeoCombinationPolyline : public GeoPolyline
{
public:
    GeoCombinationPolyline(GeographicPolylineLayer& layer, uint32 drawOrder)
        : GeoPolyline(*(layer.GetTileDataType())),
          m_isSelected(false),
          m_drawOrder(drawOrder)
    {
    }

    virtual ~GeoCombinationPolyline() {};

typedef map<GeoPolyline*, shared_ptr<GeographicLayer> > GEOGRAPHIC_LAYER_MAP;

    // Refer to Geopolyline for description.
    virtual void SetColors(shared_ptr<const vector<GeoPolylineAttributeEx*> > attrs)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->SetColors(attrs);
        }
    }

    virtual void SetWidth(uint32 width)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->SetWidth(width);
        }
    }

    virtual void Submit()
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->Submit();
        }
    }

    virtual void Show(bool bShow)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->Show(bShow);
        }
    }

    virtual void SetSelected(bool isSelected)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->SetSelected(isSelected);
        }
        m_isSelected = isSelected;
    }

    virtual bool IsSelected()
    {
        return m_isSelected;
    }

    virtual void SetDrawOrder (int order)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->first->SetDrawOrder(order);
        }
        m_drawOrder = order;
    }



    uint32 GetDrawOrder()
    {
        return m_drawOrder;
    }


    void AddPolyline(GeoPolyline* subPolyline, const shared_ptr<GeographicLayer>& layer)
    {
        m_geoGraphicMap[subPolyline] = layer;
    }

    void GraphicUpdated()
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            i->second->GraphicUpdated(i->first);
        }
    }

    void RemoveGeographic(vector<UnifiedLayerPtr>& uLayers, map<Geographic*, shared_ptr<GeographicLayer> >& geoGraphicMap)
    {
        for(GEOGRAPHIC_LAYER_MAP::iterator i = m_geoGraphicMap.begin(); i != m_geoGraphicMap.end(); ++i)
        {
            uLayers.push_back(i->second);
            geoGraphicMap.erase(i->first);
        }
        m_geoGraphicMap.clear();
    }

    void
    SetParentGeographic(Geographic* parent)
    {
        m_parentGeographic = parent;
    }

    Geographic*
    GetParentGeographic()
    {
        return m_parentGeographic;
    }

private:
    NB_DISABLE_COPY(GeoCombinationPolyline);

    bool   m_isSelected;                /*!< Flag to indicate if it is selected. */

    uint32 m_drawOrder;

    GEOGRAPHIC_LAYER_MAP m_geoGraphicMap;
};

}


/* See description in header file. */
GeoPolyline* OptionalLayersAgentImpl::AddPolyline(const GeoPolylineOption& option)
{
    GeoPolyline* polyline = NULL;
    if (m_pLayerIdGenerator && m_pContext)
    {
        GeoPolylineOptionOperation operation(option);
        shared_ptr<vector<GeoPolylineOption> > subOptions(new vector<GeoPolylineOption>());
        if(operation.isNeedCutPolyline(subOptions))
        {
            nbmap::GeoCombinationPolyline* masterPolyline = NULL;
            vector<GeoPolylineOption>::iterator it = subOptions->begin();
            for(; it != subOptions->end(); ++it)
            {
                GeographicPolylineLayerPtr layer(
                        CCC_NEW GeographicPolylineLayer(m_pContext, m_pLayerIdGenerator, this, *it));
                if (layer)
                {
                    if(!masterPolyline)
                    {
                        masterPolyline = new nbmap::GeoCombinationPolyline(*layer, it->m_zOrder);
                    }

                    polyline = layer->GetPolylineObject();
                    polyline->SetParentGeographic(masterPolyline);
                    {
                        Lock lock(m_lock);
                        m_geoGraphicMap[polyline] = layer;
                    }
                    masterPolyline->AddPolyline(polyline, layer);
                }
            }
            m_combinPolylines.insert(masterPolyline);
            masterPolyline->GraphicUpdated();
            masterPolyline->Show((true));
            return masterPolyline;
        }
        else
        {
            GeographicPolylineLayerPtr layer(
            CCC_NEW GeographicPolylineLayer(m_pContext, m_pLayerIdGenerator, this, option));
            if (layer)
            {
                polyline = layer->GetPolylineObject();
                //No need cutting polyline's parent is itself.
                polyline->SetParentGeographic(polyline);
                {
                    Lock lock(m_lock);
                    m_geoGraphicMap[polyline] = layer;
                }

                // this is not good...
                layer->GraphicUpdated(polyline);
                polyline->Show((true));
            }
            return polyline;
        }
    }

    return polyline;
}

void
OptionalLayersAgentImpl::SetPolylineSelected(GeoPolyline* polyline, bool selected)
{
    if (!polyline)
    {
        return;
    }

    polyline->SetSelected(selected);

}

void
OptionalLayersAgentImpl::SetPolylineVisible(GeoPolyline* polyline, bool visible)
{
    if (!polyline)
    {
        return;
    }
    polyline->Show(visible);

}

void
OptionalLayersAgentImpl::SetPolylineZorder(GeoPolyline* polyline, int zorder)
{
    if (!polyline)
    {
        return;
    }

    polyline->SetDrawOrder(zorder);

}

/* See description in header file. */
void OptionalLayersAgentImpl::RemoveGeographic(vector<Geographic*>& graphics)
{
    if (graphics.empty() || m_geoGraphicMap.empty())
    {
        return;
    }

    vector<UnifiedLayerPtr> uLayers;
    vector<Geographic*>::const_iterator iter = graphics.begin();
    vector<Geographic*>::const_iterator end  = graphics.end();
    {
        Lock lock(m_lock);
        for (; iter != end; ++iter)
        {
            if(m_combinPolylines.find(*iter) != m_combinPolylines.end())
            {
                static_cast<GeoCombinationPolyline*>(*iter)->RemoveGeographic(uLayers, m_geoGraphicMap);
                m_combinPolylines.erase(*iter);
                delete *iter;
            }
            GEOGRAPHIC_LAYER_MAP::iterator result = m_geoGraphicMap.find(*iter);
            if (result != m_geoGraphicMap.end() && result->second)
            {
                uLayers.push_back(result->second);
                m_geoGraphicMap.erase(result);
            }
        }
    }

    if (!uLayers.empty())
    {
        EventQueue_LayerOperationParameter* param =
                CCC_NEW EventQueue_LayerOperationParameter(
                    *this, uLayers,
                    EventQueue_LayerOperationParameter::LOT_Remove,m_isValid);
        if (param)
        {
            uint32 taskId = 0;
            PAL_Error error =
                    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_pContext),
                                          &OptionalLayersAgentImpl::CCC_LayersModified,
                                          param,
                                          &taskId);
            if (error != PAL_Ok)
            {
                delete param;
            }

        }
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::GetAllGeographics(vector<Geographic*>& graphic)
{
    // Add lock if there are issues ...
    if (!m_geoGraphicMap.empty())
    {
        graphic.reserve(m_geoGraphicMap.size());
        Lock lock(m_lock);
        for_each(m_geoGraphicMap.begin(), m_geoGraphicMap.end(), GeographicCollector(&graphic));
        graphic.swap(graphic);
    }
}

void OptionalLayersAgentImpl::GeographicUpdated(Geographic* graphic, GeographicUpdateType updateType)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    GEOGRAPHIC_LAYER_MAP::iterator result = m_geoGraphicMap.find(graphic);
    if (result == m_geoGraphicMap.end() || !result->second || !uLayerManager ||
        updateType == GUT_None)
    {
        return;
    }

    GeographicLayerPtr layer = result->second;
    vector<UnifiedLayerPtr> layers = layer->GetAllChildrenLayers();
    layers.insert(layers.begin(), layer);
    switch (updateType)
    {
        case GUT_Reload:
        {
            for_each(layers.begin(), layers.end(),
                     bind1st(mem_fun(&UnifiedLayerManager::ReloadTilesOfLayer),
                             uLayerManager));
            break;
        }
        case GUT_Refresh:
        {
            for_each(layers.begin(), layers.end(),
                     bind1st(mem_fun(&UnifiedLayerManager::RefreshTilesOfLayer),
                             uLayerManager));
            break;
        }
        default:
        {
            break;
        }
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::MaterialAdded(ExternalMaterialPtr material)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (uLayerManager)
    {
        uLayerManager->NotifyMaterialAdded(material);
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::MaterialRemoved(ExternalMaterialPtr material)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (uLayerManager)
    {
        uLayerManager->NotifyMaterialRemoved(material);
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::MaterialActivate(ExternalMaterialPtr material)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (uLayerManager)
    {
        uLayerManager->NotifyMaterialActived(material);
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::GeographicToggled(Geographic* graphic)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!graphic || !uLayerManager)
    {
        return;
    }

    GEOGRAPHIC_LAYER_MAP::iterator result;
    {
        Lock lock(m_lock);
        result = m_geoGraphicMap.find(graphic);
        if (result == m_geoGraphicMap.end() || !result->second)
        {
            return;
        }
    }

    vector<UnifiedLayerPtr> layers = result->second->GetAllChildrenLayers();
    layers.push_back(result->second);

    vector<UnifiedLayerPtr>::const_iterator iter = layers.begin();
    vector<UnifiedLayerPtr>::const_iterator end  = layers.end();
    if (result->second->IsEnabled())
    {
        vector<LayerPtr> addedLayers;
        for (; iter != end; ++iter)
        {
            if (m_pLayerProvider->AddLayer(*iter))
            {
                addedLayers.push_back(*iter);
            }
        }

        if (!addedLayers.empty())
        {
            uLayerManager->NotifyLayersAdded(addedLayers);
        }
        return;
    }

    vector<LayerPtr> removedLayers;
    for (; iter != end; ++iter)
    {
        if (m_pLayerProvider->RemoveLayer(*iter))
        {
            removedLayers.push_back(*iter);
        }
    }
    if (!removedLayers.empty())
    {
        uLayerManager->NotifyLayersRemoved(removedLayers);
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::CCC_LayersModified(PAL_Instance* pal, void* userData)
{
    EventQueue_LayerOperationParameter* param =
            static_cast<EventQueue_LayerOperationParameter*>(userData);
    if (!pal || !param )
    {
        return ;
    }

    if (param->m_isValid && *param->m_isValid)
    {
        switch (param->m_type)
        {
            case EventQueue_LayerOperationParameter::LOT_Add:
            {
                param->m_agent.UnifiedLayersAdded(param->m_layers);
                break;
            }

            case EventQueue_LayerOperationParameter::LOT_Remove:
            {
                param->m_agent.UnifiedLayersRemoved(param->m_layers);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    delete param;
}

/* See description in header file. */
void OptionalLayersAgentImpl::UnifiedLayersAdded(const vector<UnifiedLayerPtr>& layers)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());

    if (uLayerManager && !layers.empty())
    {
        vector<LayerPtr> addedLayers;
        for_each(layers.begin(), layers.end(),
                 UnifiedLayerCollector(&addedLayers, m_pLayerProvider.get(),
                                       &LocalLayerProvider::AddLayer));
        if (!addedLayers.empty())
        {
            uLayerManager->NotifyLayersAdded(addedLayers);
        }
    }
}

/* See description in header file. */
void OptionalLayersAgentImpl::UnifiedLayersRemoved(const vector<UnifiedLayerPtr>& layers)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());

    if (uLayerManager && !layers.empty())
    {
        vector<LayerPtr> removedLayers;
        for_each(layers.begin(), layers.end(),
                 UnifiedLayerCollector(&removedLayers,
                                       m_pLayerProvider.get(),
                                       &LocalLayerProvider::RemoveLayer));
        if (!removedLayers.empty())
        {
            uLayerManager->NotifyLayersRemoved(removedLayers);
        }
    }
}

/*! @} */

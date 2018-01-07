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
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#pragma once
#ifndef ROUTING_HOOKS_H
#define ROUTING_HOOKS_H

// Return codes for hook functions
#define RH_FALSE    0
#define RH_NONE     0
#define RH_TRUE     1
#define RH_OK       1
#define RH_ABORT    -1

// Event type for RouteEvent callback
#define RH_EVENT_START      0
#define RH_EVENT_ITERATION  1
#define RH_EVENT_END        2

// Report modes for RouteEvent return code
#define RH_MODE_ACTIVENODE      0x01
#define RH_MODE_ACTIVENODELIST  0x02
#define RH_MODE_ACTIVENODEPATH  0x04
#define RH_MODE_ACTIVENODEDIFF  0x08

#define RH_FLAGS_AT_REF 0x01

// An exploration node.
typedef struct ExpNode {
    char      mapId;
    char      layer;
    char      linkIdx;
    char      flags;
    int       block;
    short     nodeIdx;
    float     nodeCost;
    float     totalCost;
} ExpNode;

// Function to report significant events during routing.
typedef int (*ReportEventFn) (void * cxt, int eventid);

// Function to report node at head of active list.
typedef int (*ActiveNodeFn) (void * cxt, const ExpNode * activeNode);

// Function to report current active list.
typedef int (*ActiveNodeListFn) (void * cxt, int n, const ExpNode * activeNodeList);

// Function to report best path for head node.
typedef int (*ActiveNodePathFn) (void * cxt, int n, const ExpNode * activeNodePath);

typedef struct RoutingHooks {
    void              * cxt;         // Context passed to hook functions.
    
    ReportEventFn       reportEvent;
    ActiveNodeFn        activeNode;
    ActiveNodeListFn    activeNodeList;
    ActiveNodePathFn    activeNodePath;
    RoutingHooks () : cxt (NULL), reportEvent (NULL),
        activeNode (NULL), activeNodeList (NULL), activeNodePath (NULL) {}
} RoutingHooks;


#endif // ROUTING_HOOKS_H

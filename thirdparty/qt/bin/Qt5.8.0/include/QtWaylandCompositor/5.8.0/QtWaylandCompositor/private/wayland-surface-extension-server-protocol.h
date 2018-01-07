/* 
 * Copyright (C) 2015 The Qt Company Ltd.
 * Contact: http://www.qt.io/licensing/
 * 
 * This file is part of the plugins of the Qt Toolkit.
 * 
 * $QT_BEGIN_LICENSE:BSD$
 * You may use this file under the terms of the BSD license as follows:
 * 
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * * Neither the name of The Qt Company Ltd nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 * 
 * $QT_END_LICENSE$
 */

#ifndef SURFACE_EXTENSION_SERVER_PROTOCOL_H
#define SURFACE_EXTENSION_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct qt_extended_surface;
struct qt_surface_extension;
struct wl_surface;

extern const struct wl_interface qt_surface_extension_interface;
extern const struct wl_interface qt_extended_surface_interface;

struct qt_surface_extension_interface {
	/**
	 * get_extended_surface - (none)
	 * @id: (none)
	 * @surface: (none)
	 */
	void (*get_extended_surface)(struct wl_client *client,
				     struct wl_resource *resource,
				     uint32_t id,
				     struct wl_resource *surface);
};


#ifndef QT_EXTENDED_SURFACE_ORIENTATION_ENUM
#define QT_EXTENDED_SURFACE_ORIENTATION_ENUM
enum qt_extended_surface_orientation {
	QT_EXTENDED_SURFACE_ORIENTATION_PRIMARYORIENTATION = 0,
	QT_EXTENDED_SURFACE_ORIENTATION_PORTRAITORIENTATION = 1,
	QT_EXTENDED_SURFACE_ORIENTATION_LANDSCAPEORIENTATION = 2,
	QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDPORTRAITORIENTATION = 4,
	QT_EXTENDED_SURFACE_ORIENTATION_INVERTEDLANDSCAPEORIENTATION = 8,
};
#endif /* QT_EXTENDED_SURFACE_ORIENTATION_ENUM */

#ifndef QT_EXTENDED_SURFACE_WINDOWFLAG_ENUM
#define QT_EXTENDED_SURFACE_WINDOWFLAG_ENUM
enum qt_extended_surface_windowflag {
	QT_EXTENDED_SURFACE_WINDOWFLAG_OVERRIDESSYSTEMGESTURES = 1,
	QT_EXTENDED_SURFACE_WINDOWFLAG_STAYSONTOP = 2,
	QT_EXTENDED_SURFACE_WINDOWFLAG_BYPASSWINDOWMANAGER = 4,
};
#endif /* QT_EXTENDED_SURFACE_WINDOWFLAG_ENUM */

struct qt_extended_surface_interface {
	/**
	 * update_generic_property - (none)
	 * @name: (none)
	 * @value: (none)
	 */
	void (*update_generic_property)(struct wl_client *client,
					struct wl_resource *resource,
					const char *name,
					struct wl_array *value);
	/**
	 * set_content_orientation_mask - (none)
	 * @orientation: (none)
	 */
	void (*set_content_orientation_mask)(struct wl_client *client,
					     struct wl_resource *resource,
					     int32_t orientation);
	/**
	 * set_window_flags - (none)
	 * @flags: (none)
	 */
	void (*set_window_flags)(struct wl_client *client,
				 struct wl_resource *resource,
				 int32_t flags);
	/**
	 * raise - (none)
	 */
	void (*raise)(struct wl_client *client,
		      struct wl_resource *resource);
	/**
	 * lower - (none)
	 */
	void (*lower)(struct wl_client *client,
		      struct wl_resource *resource);
};

#define QT_EXTENDED_SURFACE_ONSCREEN_VISIBILITY	0
#define QT_EXTENDED_SURFACE_SET_GENERIC_PROPERTY	1
#define QT_EXTENDED_SURFACE_CLOSE	2

#define QT_EXTENDED_SURFACE_ONSCREEN_VISIBILITY_SINCE_VERSION	1
#define QT_EXTENDED_SURFACE_SET_GENERIC_PROPERTY_SINCE_VERSION	1
#define QT_EXTENDED_SURFACE_CLOSE_SINCE_VERSION	1

static inline void
qt_extended_surface_send_onscreen_visibility(struct wl_resource *resource_, int32_t visible)
{
	wl_resource_post_event(resource_, QT_EXTENDED_SURFACE_ONSCREEN_VISIBILITY, visible);
}

static inline void
qt_extended_surface_send_set_generic_property(struct wl_resource *resource_, const char *name, struct wl_array *value)
{
	wl_resource_post_event(resource_, QT_EXTENDED_SURFACE_SET_GENERIC_PROPERTY, name, value);
}

static inline void
qt_extended_surface_send_close(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, QT_EXTENDED_SURFACE_CLOSE);
}

#ifdef  __cplusplus
}
#endif

#endif

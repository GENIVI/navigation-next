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

#ifndef SURFACE_EXTENSION_CLIENT_PROTOCOL_H
#define SURFACE_EXTENSION_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct qt_extended_surface;
struct qt_surface_extension;
struct wl_surface;

extern const struct wl_interface qt_surface_extension_interface;
extern const struct wl_interface qt_extended_surface_interface;

#define QT_SURFACE_EXTENSION_GET_EXTENDED_SURFACE	0

static inline void
qt_surface_extension_set_user_data(struct qt_surface_extension *qt_surface_extension, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) qt_surface_extension, user_data);
}

static inline void *
qt_surface_extension_get_user_data(struct qt_surface_extension *qt_surface_extension)
{
	return wl_proxy_get_user_data((struct wl_proxy *) qt_surface_extension);
}

static inline void
qt_surface_extension_destroy(struct qt_surface_extension *qt_surface_extension)
{
	wl_proxy_destroy((struct wl_proxy *) qt_surface_extension);
}

static inline struct qt_extended_surface *
qt_surface_extension_get_extended_surface(struct qt_surface_extension *qt_surface_extension, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) qt_surface_extension,
			 QT_SURFACE_EXTENSION_GET_EXTENDED_SURFACE, &qt_extended_surface_interface, NULL, surface);

	return (struct qt_extended_surface *) id;
}

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

struct qt_extended_surface_listener {
	/**
	 * onscreen_visibility - (none)
	 * @visible: (none)
	 */
	void (*onscreen_visibility)(void *data,
				    struct qt_extended_surface *qt_extended_surface,
				    int32_t visible);
	/**
	 * set_generic_property - (none)
	 * @name: (none)
	 * @value: (none)
	 */
	void (*set_generic_property)(void *data,
				     struct qt_extended_surface *qt_extended_surface,
				     const char *name,
				     struct wl_array *value);
	/**
	 * close - (none)
	 */
	void (*close)(void *data,
		      struct qt_extended_surface *qt_extended_surface);
};

static inline int
qt_extended_surface_add_listener(struct qt_extended_surface *qt_extended_surface,
				 const struct qt_extended_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) qt_extended_surface,
				     (void (**)(void)) listener, data);
}

#define QT_EXTENDED_SURFACE_UPDATE_GENERIC_PROPERTY	0
#define QT_EXTENDED_SURFACE_SET_CONTENT_ORIENTATION_MASK	1
#define QT_EXTENDED_SURFACE_SET_WINDOW_FLAGS	2
#define QT_EXTENDED_SURFACE_RAISE	3
#define QT_EXTENDED_SURFACE_LOWER	4

static inline void
qt_extended_surface_set_user_data(struct qt_extended_surface *qt_extended_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) qt_extended_surface, user_data);
}

static inline void *
qt_extended_surface_get_user_data(struct qt_extended_surface *qt_extended_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) qt_extended_surface);
}

static inline void
qt_extended_surface_destroy(struct qt_extended_surface *qt_extended_surface)
{
	wl_proxy_destroy((struct wl_proxy *) qt_extended_surface);
}

static inline void
qt_extended_surface_update_generic_property(struct qt_extended_surface *qt_extended_surface, const char *name, struct wl_array *value)
{
	wl_proxy_marshal((struct wl_proxy *) qt_extended_surface,
			 QT_EXTENDED_SURFACE_UPDATE_GENERIC_PROPERTY, name, value);
}

static inline void
qt_extended_surface_set_content_orientation_mask(struct qt_extended_surface *qt_extended_surface, int32_t orientation)
{
	wl_proxy_marshal((struct wl_proxy *) qt_extended_surface,
			 QT_EXTENDED_SURFACE_SET_CONTENT_ORIENTATION_MASK, orientation);
}

static inline void
qt_extended_surface_set_window_flags(struct qt_extended_surface *qt_extended_surface, int32_t flags)
{
	wl_proxy_marshal((struct wl_proxy *) qt_extended_surface,
			 QT_EXTENDED_SURFACE_SET_WINDOW_FLAGS, flags);
}

static inline void
qt_extended_surface_raise(struct qt_extended_surface *qt_extended_surface)
{
	wl_proxy_marshal((struct wl_proxy *) qt_extended_surface,
			 QT_EXTENDED_SURFACE_RAISE);
}

static inline void
qt_extended_surface_lower(struct qt_extended_surface *qt_extended_surface)
{
	wl_proxy_marshal((struct wl_proxy *) qt_extended_surface,
			 QT_EXTENDED_SURFACE_LOWER);
}

#ifdef  __cplusplus
}
#endif

#endif

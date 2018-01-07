#ifndef QT_WAYLAND_SERVER_SURFACE_EXTENSION
#define QT_WAYLAND_SERVER_SURFACE_EXTENSION

#include "wayland-server.h"
#include <QtWaylandCompositor/private/wayland-surface-extension-server-protocol.h>
#include <QByteArray>
#include <QMultiMap>
#include <QString>

#ifndef WAYLAND_VERSION_CHECK
#define WAYLAND_VERSION_CHECK(major, minor, micro) \
    ((WAYLAND_VERSION_MAJOR > (major)) || \
    (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR > (minor)) || \
    (WAYLAND_VERSION_MAJOR == (major) && WAYLAND_VERSION_MINOR == (minor) && WAYLAND_VERSION_MICRO >= (micro)))
#endif

QT_BEGIN_NAMESPACE

#if !defined(Q_WAYLAND_SERVER_SURFACE_EXTENSION_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_SERVER_SURFACE_EXTENSION_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_SERVER_SURFACE_EXTENSION_EXPORT
#  endif
#endif

namespace QtWaylandServer {
    class Q_WAYLAND_SERVER_SURFACE_EXTENSION_EXPORT qt_surface_extension
    {
    public:
        qt_surface_extension(struct ::wl_client *client, int id, int version);
        qt_surface_extension(struct ::wl_display *display, int version);
        qt_surface_extension(struct ::wl_resource *resource);
        qt_surface_extension();

        virtual ~qt_surface_extension();

        class Resource
        {
        public:
            Resource() : surface_extension_object(0), handle(0) {}
            virtual ~Resource() {}

            qt_surface_extension *surface_extension_object;
            struct ::wl_resource *handle;

            struct ::wl_client *client() const { return handle->client; }
            int version() const { return wl_resource_get_version(handle); }

            static Resource *fromResource(struct ::wl_resource *resource);
        };

        void init(struct ::wl_client *client, int id, int version);
        void init(struct ::wl_display *display, int version);
        void init(struct ::wl_resource *resource);

        Resource *add(struct ::wl_client *client, int version);
        Resource *add(struct ::wl_client *client, int id, int version);
        Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);

        Resource *resource() { return m_resource; }
        const Resource *resource() const { return m_resource; }

        QMultiMap<struct ::wl_client*, Resource*> resourceMap() { return m_resource_map; }
        const QMultiMap<struct ::wl_client*, Resource*> resourceMap() const { return m_resource_map; }

        bool isGlobal() const { return m_global != 0; }
        bool isResource() const { return m_resource != 0; }

        static const struct ::wl_interface *interface();
        static QByteArray interfaceName() { return interface()->name; }
        static int interfaceVersion() { return interface()->version; }


    protected:
        virtual Resource *surface_extension_allocate();

        virtual void surface_extension_bind_resource(Resource *resource);
        virtual void surface_extension_destroy_resource(Resource *resource);

        virtual void surface_extension_get_extended_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::qt_surface_extension_interface m_qt_surface_extension_interface;

        static void handle_get_extended_surface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_SURFACE_EXTENSION_EXPORT qt_extended_surface
    {
    public:
        qt_extended_surface(struct ::wl_client *client, int id, int version);
        qt_extended_surface(struct ::wl_display *display, int version);
        qt_extended_surface(struct ::wl_resource *resource);
        qt_extended_surface();

        virtual ~qt_extended_surface();

        class Resource
        {
        public:
            Resource() : extended_surface_object(0), handle(0) {}
            virtual ~Resource() {}

            qt_extended_surface *extended_surface_object;
            struct ::wl_resource *handle;

            struct ::wl_client *client() const { return handle->client; }
            int version() const { return wl_resource_get_version(handle); }

            static Resource *fromResource(struct ::wl_resource *resource);
        };

        void init(struct ::wl_client *client, int id, int version);
        void init(struct ::wl_display *display, int version);
        void init(struct ::wl_resource *resource);

        Resource *add(struct ::wl_client *client, int version);
        Resource *add(struct ::wl_client *client, int id, int version);
        Resource *add(struct wl_list *resource_list, struct ::wl_client *client, int id, int version);

        Resource *resource() { return m_resource; }
        const Resource *resource() const { return m_resource; }

        QMultiMap<struct ::wl_client*, Resource*> resourceMap() { return m_resource_map; }
        const QMultiMap<struct ::wl_client*, Resource*> resourceMap() const { return m_resource_map; }

        bool isGlobal() const { return m_global != 0; }
        bool isResource() const { return m_resource != 0; }

        static const struct ::wl_interface *interface();
        static QByteArray interfaceName() { return interface()->name; }
        static int interfaceVersion() { return interface()->version; }


        enum orientation {
            orientation_PrimaryOrientation = 0,
            orientation_PortraitOrientation = 1,
            orientation_LandscapeOrientation = 2,
            orientation_InvertedPortraitOrientation = 4,
            orientation_InvertedLandscapeOrientation = 8
        };

        enum windowflag {
            windowflag_OverridesSystemGestures = 1,
            windowflag_StaysOnTop = 2,
            windowflag_BypassWindowManager = 4
        };

        void send_onscreen_visibility(int32_t visible);
        void send_onscreen_visibility(struct ::wl_resource *resource, int32_t visible);
        void send_set_generic_property(const QString &name, const QByteArray &value);
        void send_set_generic_property(struct ::wl_resource *resource, const QString &name, const QByteArray &value);
        void send_close();
        void send_close(struct ::wl_resource *resource);

    protected:
        virtual Resource *extended_surface_allocate();

        virtual void extended_surface_bind_resource(Resource *resource);
        virtual void extended_surface_destroy_resource(Resource *resource);

        virtual void extended_surface_update_generic_property(Resource *resource, const QString &name, wl_array *value);
        virtual void extended_surface_set_content_orientation_mask(Resource *resource, int32_t orientation);
        virtual void extended_surface_set_window_flags(Resource *resource, int32_t flags);
        virtual void extended_surface_raise(Resource *resource);
        virtual void extended_surface_lower(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::qt_extended_surface_interface m_qt_extended_surface_interface;

        static void handle_update_generic_property(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *name,
            wl_array *value);
        static void handle_set_content_orientation_mask(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t orientation);
        static void handle_set_window_flags(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t flags);
        static void handle_raise(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_lower(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };
}

QT_END_NAMESPACE

#endif

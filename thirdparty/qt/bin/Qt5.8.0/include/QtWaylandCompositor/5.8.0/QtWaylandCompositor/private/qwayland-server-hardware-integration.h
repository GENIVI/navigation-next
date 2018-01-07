#ifndef QT_WAYLAND_SERVER_HARDWARE_INTEGRATION
#define QT_WAYLAND_SERVER_HARDWARE_INTEGRATION

#include "wayland-server.h"
#include <QtWaylandCompositor/private/wayland-hardware-integration-server-protocol.h>
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

#if !defined(Q_WAYLAND_SERVER_HARDWARE_INTEGRATION_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_SERVER_HARDWARE_INTEGRATION_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_SERVER_HARDWARE_INTEGRATION_EXPORT
#  endif
#endif

namespace QtWaylandServer {
    class Q_WAYLAND_SERVER_HARDWARE_INTEGRATION_EXPORT qt_hardware_integration
    {
    public:
        qt_hardware_integration(struct ::wl_client *client, int id, int version);
        qt_hardware_integration(struct ::wl_display *display, int version);
        qt_hardware_integration(struct ::wl_resource *resource);
        qt_hardware_integration();

        virtual ~qt_hardware_integration();

        class Resource
        {
        public:
            Resource() : hardware_integration_object(0), handle(0) {}
            virtual ~Resource() {}

            qt_hardware_integration *hardware_integration_object;
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


        void send_client_backend(const QString &name);
        void send_client_backend(struct ::wl_resource *resource, const QString &name);
        void send_server_backend(const QString &name);
        void send_server_backend(struct ::wl_resource *resource, const QString &name);

    protected:
        virtual Resource *hardware_integration_allocate();

        virtual void hardware_integration_bind_resource(Resource *resource);
        virtual void hardware_integration_destroy_resource(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };
}

QT_END_NAMESPACE

#endif

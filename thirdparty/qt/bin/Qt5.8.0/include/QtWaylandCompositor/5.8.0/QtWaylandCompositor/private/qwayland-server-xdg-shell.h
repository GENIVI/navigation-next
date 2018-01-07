#ifndef QT_WAYLAND_SERVER_XDG_SHELL
#define QT_WAYLAND_SERVER_XDG_SHELL

#include "wayland-server.h"
#include <QtWaylandCompositor/private/wayland-xdg-shell-server-protocol.h>
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

#if !defined(Q_WAYLAND_SERVER_XDG_SHELL_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_SERVER_XDG_SHELL_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_SERVER_XDG_SHELL_EXPORT
#  endif
#endif

namespace QtWaylandServer {
    class Q_WAYLAND_SERVER_XDG_SHELL_EXPORT xdg_shell
    {
    public:
        xdg_shell(struct ::wl_client *client, int id, int version);
        xdg_shell(struct ::wl_display *display, int version);
        xdg_shell(struct ::wl_resource *resource);
        xdg_shell();

        virtual ~xdg_shell();

        class Resource
        {
        public:
            Resource() : xdg_shell_object(0), handle(0) {}
            virtual ~Resource() {}

            xdg_shell *xdg_shell_object;
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


        enum version {
            version_current = 5 // Always the latest version
        };

        enum error {
            error_role = 0, // given wl_surface has another role
            error_defunct_surfaces = 1, // xdg_shell was destroyed before children
            error_not_the_topmost_popup = 2, // the client tried to map or destroy a non-topmost popup
            error_invalid_popup_parent = 3 // the client specified an invalid popup parent surface
        };

        void send_ping(uint32_t serial);
        void send_ping(struct ::wl_resource *resource, uint32_t serial);

    protected:
        virtual Resource *xdg_shell_allocate();

        virtual void xdg_shell_bind_resource(Resource *resource);
        virtual void xdg_shell_destroy_resource(Resource *resource);

        virtual void xdg_shell_destroy(Resource *resource);
        virtual void xdg_shell_use_unstable_version(Resource *resource, int32_t version);
        virtual void xdg_shell_get_xdg_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface);
        virtual void xdg_shell_get_xdg_popup(Resource *resource, uint32_t id, struct ::wl_resource *surface, struct ::wl_resource *parent, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y);
        virtual void xdg_shell_pong(Resource *resource, uint32_t serial);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_shell_interface m_xdg_shell_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_use_unstable_version(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t version);
        static void handle_get_xdg_surface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface);
        static void handle_get_xdg_popup(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface,
            struct ::wl_resource *parent,
            struct ::wl_resource *seat,
            uint32_t serial,
            int32_t x,
            int32_t y);
        static void handle_pong(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_XDG_SHELL_EXPORT xdg_surface
    {
    public:
        xdg_surface(struct ::wl_client *client, int id, int version);
        xdg_surface(struct ::wl_display *display, int version);
        xdg_surface(struct ::wl_resource *resource);
        xdg_surface();

        virtual ~xdg_surface();

        class Resource
        {
        public:
            Resource() : xdg_surface_object(0), handle(0) {}
            virtual ~Resource() {}

            xdg_surface *xdg_surface_object;
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


        enum resize_edge {
            resize_edge_none = 0,
            resize_edge_top = 1,
            resize_edge_bottom = 2,
            resize_edge_left = 4,
            resize_edge_top_left = 5,
            resize_edge_bottom_left = 6,
            resize_edge_right = 8,
            resize_edge_top_right = 9,
            resize_edge_bottom_right = 10
        };

        enum state {
            state_maximized = 1, // the surface is maximized
            state_fullscreen = 2, // the surface is fullscreen
            state_resizing = 3,
            state_activated = 4
        };

        void send_configure(int32_t width, int32_t height, const QByteArray &states, uint32_t serial);
        void send_configure(struct ::wl_resource *resource, int32_t width, int32_t height, const QByteArray &states, uint32_t serial);
        void send_close();
        void send_close(struct ::wl_resource *resource);

    protected:
        virtual Resource *xdg_surface_allocate();

        virtual void xdg_surface_bind_resource(Resource *resource);
        virtual void xdg_surface_destroy_resource(Resource *resource);

        virtual void xdg_surface_destroy(Resource *resource);
        virtual void xdg_surface_set_parent(Resource *resource, struct ::wl_resource *parent);
        virtual void xdg_surface_set_title(Resource *resource, const QString &title);
        virtual void xdg_surface_set_app_id(Resource *resource, const QString &app_id);
        virtual void xdg_surface_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y);
        virtual void xdg_surface_move(Resource *resource, struct ::wl_resource *seat, uint32_t serial);
        virtual void xdg_surface_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges);
        virtual void xdg_surface_ack_configure(Resource *resource, uint32_t serial);
        virtual void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
        virtual void xdg_surface_set_maximized(Resource *resource);
        virtual void xdg_surface_unset_maximized(Resource *resource);
        virtual void xdg_surface_set_fullscreen(Resource *resource, struct ::wl_resource *output);
        virtual void xdg_surface_unset_fullscreen(Resource *resource);
        virtual void xdg_surface_set_minimized(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_surface_interface m_xdg_surface_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_parent(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *parent);
        static void handle_set_title(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *title);
        static void handle_set_app_id(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *app_id);
        static void handle_show_window_menu(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial,
            int32_t x,
            int32_t y);
        static void handle_move(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial);
        static void handle_resize(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial,
            uint32_t edges);
        static void handle_ack_configure(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial);
        static void handle_set_window_geometry(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);
        static void handle_set_maximized(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_unset_maximized(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_fullscreen(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *output);
        static void handle_unset_fullscreen(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_minimized(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_XDG_SHELL_EXPORT xdg_popup
    {
    public:
        xdg_popup(struct ::wl_client *client, int id, int version);
        xdg_popup(struct ::wl_display *display, int version);
        xdg_popup(struct ::wl_resource *resource);
        xdg_popup();

        virtual ~xdg_popup();

        class Resource
        {
        public:
            Resource() : xdg_popup_object(0), handle(0) {}
            virtual ~Resource() {}

            xdg_popup *xdg_popup_object;
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


        void send_popup_done();
        void send_popup_done(struct ::wl_resource *resource);

    protected:
        virtual Resource *xdg_popup_allocate();

        virtual void xdg_popup_bind_resource(Resource *resource);
        virtual void xdg_popup_destroy_resource(Resource *resource);

        virtual void xdg_popup_destroy(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::xdg_popup_interface m_xdg_popup_interface;

        static void handle_destroy(
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

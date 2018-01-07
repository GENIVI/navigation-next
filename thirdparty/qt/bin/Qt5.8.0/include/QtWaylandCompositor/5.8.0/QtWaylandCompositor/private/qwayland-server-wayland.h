#ifndef QT_WAYLAND_SERVER_WAYLAND
#define QT_WAYLAND_SERVER_WAYLAND

#include "wayland-server.h"
#include <QtWaylandCompositor/private/wayland-wayland-server-protocol.h>
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

#if !defined(Q_WAYLAND_SERVER_WAYLAND_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_SERVER_WAYLAND_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_SERVER_WAYLAND_EXPORT
#  endif
#endif

namespace QtWaylandServer {
    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_callback
    {
    public:
        wl_callback(struct ::wl_client *client, int id, int version);
        wl_callback(struct ::wl_display *display, int version);
        wl_callback(struct ::wl_resource *resource);
        wl_callback();

        virtual ~wl_callback();

        class Resource
        {
        public:
            Resource() : callback_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_callback *callback_object;
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


        void send_done(uint32_t callback_data);
        void send_done(struct ::wl_resource *resource, uint32_t callback_data);

    protected:
        virtual Resource *callback_allocate();

        virtual void callback_bind_resource(Resource *resource);
        virtual void callback_destroy_resource(Resource *resource);

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

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_compositor
    {
    public:
        wl_compositor(struct ::wl_client *client, int id, int version);
        wl_compositor(struct ::wl_display *display, int version);
        wl_compositor(struct ::wl_resource *resource);
        wl_compositor();

        virtual ~wl_compositor();

        class Resource
        {
        public:
            Resource() : compositor_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_compositor *compositor_object;
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
        virtual Resource *compositor_allocate();

        virtual void compositor_bind_resource(Resource *resource);
        virtual void compositor_destroy_resource(Resource *resource);

        virtual void compositor_create_surface(Resource *resource, uint32_t id);
        virtual void compositor_create_region(Resource *resource, uint32_t id);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_compositor_interface m_wl_compositor_interface;

        static void handle_create_surface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);
        static void handle_create_region(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_shm_pool
    {
    public:
        wl_shm_pool(struct ::wl_client *client, int id, int version);
        wl_shm_pool(struct ::wl_display *display, int version);
        wl_shm_pool(struct ::wl_resource *resource);
        wl_shm_pool();

        virtual ~wl_shm_pool();

        class Resource
        {
        public:
            Resource() : shm_pool_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_shm_pool *shm_pool_object;
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
        virtual Resource *shm_pool_allocate();

        virtual void shm_pool_bind_resource(Resource *resource);
        virtual void shm_pool_destroy_resource(Resource *resource);

        virtual void shm_pool_create_buffer(Resource *resource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format);
        virtual void shm_pool_destroy(Resource *resource);
        virtual void shm_pool_resize(Resource *resource, int32_t size);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_shm_pool_interface m_wl_shm_pool_interface;

        static void handle_create_buffer(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            int32_t offset,
            int32_t width,
            int32_t height,
            int32_t stride,
            uint32_t format);
        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_resize(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t size);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_shm
    {
    public:
        wl_shm(struct ::wl_client *client, int id, int version);
        wl_shm(struct ::wl_display *display, int version);
        wl_shm(struct ::wl_resource *resource);
        wl_shm();

        virtual ~wl_shm();

        class Resource
        {
        public:
            Resource() : shm_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_shm *shm_object;
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


        enum error {
            error_invalid_format = 0, // buffer format is not known
            error_invalid_stride = 1, // invalid size or stride during pool or buffer creation
            error_invalid_fd = 2 // mmapping the file descriptor failed
        };

        enum format {
            format_argb8888 = 0, // 32-bit ARGB format
            format_xrgb8888 = 1, // 32-bit RGB format
            format_c8 = 0x20203843,
            format_rgb332 = 0x38424752,
            format_bgr233 = 0x38524742,
            format_xrgb4444 = 0x32315258,
            format_xbgr4444 = 0x32314258,
            format_rgbx4444 = 0x32315852,
            format_bgrx4444 = 0x32315842,
            format_argb4444 = 0x32315241,
            format_abgr4444 = 0x32314241,
            format_rgba4444 = 0x32314152,
            format_bgra4444 = 0x32314142,
            format_xrgb1555 = 0x35315258,
            format_xbgr1555 = 0x35314258,
            format_rgbx5551 = 0x35315852,
            format_bgrx5551 = 0x35315842,
            format_argb1555 = 0x35315241,
            format_abgr1555 = 0x35314241,
            format_rgba5551 = 0x35314152,
            format_bgra5551 = 0x35314142,
            format_rgb565 = 0x36314752,
            format_bgr565 = 0x36314742,
            format_rgb888 = 0x34324752,
            format_bgr888 = 0x34324742,
            format_xbgr8888 = 0x34324258,
            format_rgbx8888 = 0x34325852,
            format_bgrx8888 = 0x34325842,
            format_abgr8888 = 0x34324241,
            format_rgba8888 = 0x34324152,
            format_bgra8888 = 0x34324142,
            format_xrgb2101010 = 0x30335258,
            format_xbgr2101010 = 0x30334258,
            format_rgbx1010102 = 0x30335852,
            format_bgrx1010102 = 0x30335842,
            format_argb2101010 = 0x30335241,
            format_abgr2101010 = 0x30334241,
            format_rgba1010102 = 0x30334152,
            format_bgra1010102 = 0x30334142,
            format_yuyv = 0x56595559,
            format_yvyu = 0x55595659,
            format_uyvy = 0x59565955,
            format_vyuy = 0x59555956,
            format_ayuv = 0x56555941,
            format_nv12 = 0x3231564e,
            format_nv21 = 0x3132564e,
            format_nv16 = 0x3631564e,
            format_nv61 = 0x3136564e,
            format_yuv410 = 0x39565559,
            format_yvu410 = 0x39555659,
            format_yuv411 = 0x31315559,
            format_yvu411 = 0x31315659,
            format_yuv420 = 0x32315559,
            format_yvu420 = 0x32315659,
            format_yuv422 = 0x36315559,
            format_yvu422 = 0x36315659,
            format_yuv444 = 0x34325559,
            format_yvu444 = 0x34325659
        };

        void send_format(uint32_t format);
        void send_format(struct ::wl_resource *resource, uint32_t format);

    protected:
        virtual Resource *shm_allocate();

        virtual void shm_bind_resource(Resource *resource);
        virtual void shm_destroy_resource(Resource *resource);

        virtual void shm_create_pool(Resource *resource, uint32_t id, int32_t fd, int32_t size);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_shm_interface m_wl_shm_interface;

        static void handle_create_pool(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            int32_t fd,
            int32_t size);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_buffer
    {
    public:
        wl_buffer(struct ::wl_client *client, int id, int version);
        wl_buffer(struct ::wl_display *display, int version);
        wl_buffer(struct ::wl_resource *resource);
        wl_buffer();

        virtual ~wl_buffer();

        class Resource
        {
        public:
            Resource() : buffer_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_buffer *buffer_object;
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


        void send_release();
        void send_release(struct ::wl_resource *resource);

    protected:
        virtual Resource *buffer_allocate();

        virtual void buffer_bind_resource(Resource *resource);
        virtual void buffer_destroy_resource(Resource *resource);

        virtual void buffer_destroy(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_buffer_interface m_wl_buffer_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_data_offer
    {
    public:
        wl_data_offer(struct ::wl_client *client, int id, int version);
        wl_data_offer(struct ::wl_display *display, int version);
        wl_data_offer(struct ::wl_resource *resource);
        wl_data_offer();

        virtual ~wl_data_offer();

        class Resource
        {
        public:
            Resource() : data_offer_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_data_offer *data_offer_object;
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


        void send_offer(const QString &mime_type);
        void send_offer(struct ::wl_resource *resource, const QString &mime_type);

    protected:
        virtual Resource *data_offer_allocate();

        virtual void data_offer_bind_resource(Resource *resource);
        virtual void data_offer_destroy_resource(Resource *resource);

        virtual void data_offer_accept(Resource *resource, uint32_t serial, const QString &mime_type);
        virtual void data_offer_receive(Resource *resource, const QString &mime_type, int32_t fd);
        virtual void data_offer_destroy(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_data_offer_interface m_wl_data_offer_interface;

        static void handle_accept(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial,
            const char *mime_type);
        static void handle_receive(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *mime_type,
            int32_t fd);
        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_data_source
    {
    public:
        wl_data_source(struct ::wl_client *client, int id, int version);
        wl_data_source(struct ::wl_display *display, int version);
        wl_data_source(struct ::wl_resource *resource);
        wl_data_source();

        virtual ~wl_data_source();

        class Resource
        {
        public:
            Resource() : data_source_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_data_source *data_source_object;
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


        void send_target(const QString &mime_type);
        void send_target(struct ::wl_resource *resource, const QString &mime_type);
        void send_send(const QString &mime_type, int32_t fd);
        void send_send(struct ::wl_resource *resource, const QString &mime_type, int32_t fd);
        void send_cancelled();
        void send_cancelled(struct ::wl_resource *resource);

    protected:
        virtual Resource *data_source_allocate();

        virtual void data_source_bind_resource(Resource *resource);
        virtual void data_source_destroy_resource(Resource *resource);

        virtual void data_source_offer(Resource *resource, const QString &mime_type);
        virtual void data_source_destroy(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_data_source_interface m_wl_data_source_interface;

        static void handle_offer(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *mime_type);
        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_data_device
    {
    public:
        wl_data_device(struct ::wl_client *client, int id, int version);
        wl_data_device(struct ::wl_display *display, int version);
        wl_data_device(struct ::wl_resource *resource);
        wl_data_device();

        virtual ~wl_data_device();

        class Resource
        {
        public:
            Resource() : data_device_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_data_device *data_device_object;
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


        void send_data_offer(struct ::wl_resource *id);
        void send_data_offer(struct ::wl_resource *resource, struct ::wl_resource *id);
        void send_enter(uint32_t serial, struct ::wl_resource *surface, wl_fixed_t x, wl_fixed_t y, struct ::wl_resource *id);
        void send_enter(struct ::wl_resource *resource, uint32_t serial, struct ::wl_resource *surface, wl_fixed_t x, wl_fixed_t y, struct ::wl_resource *id);
        void send_leave();
        void send_leave(struct ::wl_resource *resource);
        void send_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y);
        void send_motion(struct ::wl_resource *resource, uint32_t time, wl_fixed_t x, wl_fixed_t y);
        void send_drop();
        void send_drop(struct ::wl_resource *resource);
        void send_selection(struct ::wl_resource *id);
        void send_selection(struct ::wl_resource *resource, struct ::wl_resource *id);

    protected:
        virtual Resource *data_device_allocate();

        virtual void data_device_bind_resource(Resource *resource);
        virtual void data_device_destroy_resource(Resource *resource);

        virtual void data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial);
        virtual void data_device_set_selection(Resource *resource, struct ::wl_resource *source, uint32_t serial);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_data_device_interface m_wl_data_device_interface;

        static void handle_start_drag(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *source,
            struct ::wl_resource *origin,
            struct ::wl_resource *icon,
            uint32_t serial);
        static void handle_set_selection(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *source,
            uint32_t serial);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_data_device_manager
    {
    public:
        wl_data_device_manager(struct ::wl_client *client, int id, int version);
        wl_data_device_manager(struct ::wl_display *display, int version);
        wl_data_device_manager(struct ::wl_resource *resource);
        wl_data_device_manager();

        virtual ~wl_data_device_manager();

        class Resource
        {
        public:
            Resource() : data_device_manager_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_data_device_manager *data_device_manager_object;
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
        virtual Resource *data_device_manager_allocate();

        virtual void data_device_manager_bind_resource(Resource *resource);
        virtual void data_device_manager_destroy_resource(Resource *resource);

        virtual void data_device_manager_create_data_source(Resource *resource, uint32_t id);
        virtual void data_device_manager_get_data_device(Resource *resource, uint32_t id, struct ::wl_resource *seat);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_data_device_manager_interface m_wl_data_device_manager_interface;

        static void handle_create_data_source(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);
        static void handle_get_data_device(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *seat);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_shell
    {
    public:
        wl_shell(struct ::wl_client *client, int id, int version);
        wl_shell(struct ::wl_display *display, int version);
        wl_shell(struct ::wl_resource *resource);
        wl_shell();

        virtual ~wl_shell();

        class Resource
        {
        public:
            Resource() : shell_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_shell *shell_object;
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
        virtual Resource *shell_allocate();

        virtual void shell_bind_resource(Resource *resource);
        virtual void shell_destroy_resource(Resource *resource);

        virtual void shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_shell_interface m_wl_shell_interface;

        static void handle_get_shell_surface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_shell_surface
    {
    public:
        wl_shell_surface(struct ::wl_client *client, int id, int version);
        wl_shell_surface(struct ::wl_display *display, int version);
        wl_shell_surface(struct ::wl_resource *resource);
        wl_shell_surface();

        virtual ~wl_shell_surface();

        class Resource
        {
        public:
            Resource() : shell_surface_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_shell_surface *shell_surface_object;
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


        enum resize {
            resize_none = 0,
            resize_top = 1,
            resize_bottom = 2,
            resize_left = 4,
            resize_top_left = 5,
            resize_bottom_left = 6,
            resize_right = 8,
            resize_top_right = 9,
            resize_bottom_right = 10
        };

        enum transient {
            transient_inactive = 0x1 // do not set keyboard focus
        };

        enum fullscreen_method {
            fullscreen_method_default = 0, // no preference, apply default policy
            fullscreen_method_scale = 1, // scale, preserve the surface's aspect ratio and center on output
            fullscreen_method_driver = 2, // switch output mode to the smallest mode that can fit the surface, add black borders to compensate size mismatch
            fullscreen_method_fill = 3 // no upscaling, center on output and add black borders to compensate size mismatch
        };

        void send_ping(uint32_t serial);
        void send_ping(struct ::wl_resource *resource, uint32_t serial);
        void send_configure(uint32_t edges, int32_t width, int32_t height);
        void send_configure(struct ::wl_resource *resource, uint32_t edges, int32_t width, int32_t height);
        void send_popup_done();
        void send_popup_done(struct ::wl_resource *resource);

    protected:
        virtual Resource *shell_surface_allocate();

        virtual void shell_surface_bind_resource(Resource *resource);
        virtual void shell_surface_destroy_resource(Resource *resource);

        virtual void shell_surface_pong(Resource *resource, uint32_t serial);
        virtual void shell_surface_move(Resource *resource, struct ::wl_resource *seat, uint32_t serial);
        virtual void shell_surface_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges);
        virtual void shell_surface_set_toplevel(Resource *resource);
        virtual void shell_surface_set_transient(Resource *resource, struct ::wl_resource *parent, int32_t x, int32_t y, uint32_t flags);
        virtual void shell_surface_set_fullscreen(Resource *resource, uint32_t method, uint32_t framerate, struct ::wl_resource *output);
        virtual void shell_surface_set_popup(Resource *resource, struct ::wl_resource *seat, uint32_t serial, struct ::wl_resource *parent, int32_t x, int32_t y, uint32_t flags);
        virtual void shell_surface_set_maximized(Resource *resource, struct ::wl_resource *output);
        virtual void shell_surface_set_title(Resource *resource, const QString &title);
        virtual void shell_surface_set_class(Resource *resource, const QString &class_);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_shell_surface_interface m_wl_shell_surface_interface;

        static void handle_pong(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial);
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
        static void handle_set_toplevel(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_transient(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *parent,
            int32_t x,
            int32_t y,
            uint32_t flags);
        static void handle_set_fullscreen(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t method,
            uint32_t framerate,
            struct ::wl_resource *output);
        static void handle_set_popup(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *seat,
            uint32_t serial,
            struct ::wl_resource *parent,
            int32_t x,
            int32_t y,
            uint32_t flags);
        static void handle_set_maximized(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *output);
        static void handle_set_title(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *title);
        static void handle_set_class(
            ::wl_client *client,
            struct wl_resource *resource,
            const char *class_);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_surface
    {
    public:
        wl_surface(struct ::wl_client *client, int id, int version);
        wl_surface(struct ::wl_display *display, int version);
        wl_surface(struct ::wl_resource *resource);
        wl_surface();

        virtual ~wl_surface();

        class Resource
        {
        public:
            Resource() : surface_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_surface *surface_object;
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


        enum error {
            error_invalid_scale = 0, // buffer scale value is invalid
            error_invalid_transform = 1 // buffer transform value is invalid
        };

        void send_enter(struct ::wl_resource *output);
        void send_enter(struct ::wl_resource *resource, struct ::wl_resource *output);
        void send_leave(struct ::wl_resource *output);
        void send_leave(struct ::wl_resource *resource, struct ::wl_resource *output);

    protected:
        virtual Resource *surface_allocate();

        virtual void surface_bind_resource(Resource *resource);
        virtual void surface_destroy_resource(Resource *resource);

        virtual void surface_destroy(Resource *resource);
        virtual void surface_attach(Resource *resource, struct ::wl_resource *buffer, int32_t x, int32_t y);
        virtual void surface_damage(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
        virtual void surface_frame(Resource *resource, uint32_t callback);
        virtual void surface_set_opaque_region(Resource *resource, struct ::wl_resource *region);
        virtual void surface_set_input_region(Resource *resource, struct ::wl_resource *region);
        virtual void surface_commit(Resource *resource);
        virtual void surface_set_buffer_transform(Resource *resource, int32_t transform);
        virtual void surface_set_buffer_scale(Resource *resource, int32_t scale);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_surface_interface m_wl_surface_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_attach(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *buffer,
            int32_t x,
            int32_t y);
        static void handle_damage(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);
        static void handle_frame(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t callback);
        static void handle_set_opaque_region(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *region);
        static void handle_set_input_region(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *region);
        static void handle_commit(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_buffer_transform(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t transform);
        static void handle_set_buffer_scale(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t scale);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_seat
    {
    public:
        wl_seat(struct ::wl_client *client, int id, int version);
        wl_seat(struct ::wl_display *display, int version);
        wl_seat(struct ::wl_resource *resource);
        wl_seat();

        virtual ~wl_seat();

        class Resource
        {
        public:
            Resource() : seat_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_seat *seat_object;
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


        enum capability {
            capability_pointer = 1, // The seat has pointer devices
            capability_keyboard = 2, // The seat has one or more keyboards
            capability_touch = 4 // The seat has touch devices
        };

        void send_capabilities(uint32_t capabilities);
        void send_capabilities(struct ::wl_resource *resource, uint32_t capabilities);
        void send_name(const QString &name);
        void send_name(struct ::wl_resource *resource, const QString &name);

    protected:
        virtual Resource *seat_allocate();

        virtual void seat_bind_resource(Resource *resource);
        virtual void seat_destroy_resource(Resource *resource);

        virtual void seat_get_pointer(Resource *resource, uint32_t id);
        virtual void seat_get_keyboard(Resource *resource, uint32_t id);
        virtual void seat_get_touch(Resource *resource, uint32_t id);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_seat_interface m_wl_seat_interface;

        static void handle_get_pointer(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);
        static void handle_get_keyboard(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);
        static void handle_get_touch(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_pointer
    {
    public:
        wl_pointer(struct ::wl_client *client, int id, int version);
        wl_pointer(struct ::wl_display *display, int version);
        wl_pointer(struct ::wl_resource *resource);
        wl_pointer();

        virtual ~wl_pointer();

        class Resource
        {
        public:
            Resource() : pointer_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_pointer *pointer_object;
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


        enum button_state {
            button_state_released = 0, // The button is not pressed
            button_state_pressed = 1 // The button is pressed
        };

        enum axis {
            axis_vertical_scroll = 0,
            axis_horizontal_scroll = 1
        };

        void send_enter(uint32_t serial, struct ::wl_resource *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        void send_enter(struct ::wl_resource *resource, uint32_t serial, struct ::wl_resource *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        void send_leave(uint32_t serial, struct ::wl_resource *surface);
        void send_leave(struct ::wl_resource *resource, uint32_t serial, struct ::wl_resource *surface);
        void send_motion(uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
        void send_motion(struct ::wl_resource *resource, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
        void send_button(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        void send_button(struct ::wl_resource *resource, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        void send_axis(uint32_t time, uint32_t axis, wl_fixed_t value);
        void send_axis(struct ::wl_resource *resource, uint32_t time, uint32_t axis, wl_fixed_t value);

    protected:
        virtual Resource *pointer_allocate();

        virtual void pointer_bind_resource(Resource *resource);
        virtual void pointer_destroy_resource(Resource *resource);

        virtual void pointer_set_cursor(Resource *resource, uint32_t serial, struct ::wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y);
        virtual void pointer_release(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_pointer_interface m_wl_pointer_interface;

        static void handle_set_cursor(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t serial,
            struct ::wl_resource *surface,
            int32_t hotspot_x,
            int32_t hotspot_y);
        static void handle_release(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_keyboard
    {
    public:
        wl_keyboard(struct ::wl_client *client, int id, int version);
        wl_keyboard(struct ::wl_display *display, int version);
        wl_keyboard(struct ::wl_resource *resource);
        wl_keyboard();

        virtual ~wl_keyboard();

        class Resource
        {
        public:
            Resource() : keyboard_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_keyboard *keyboard_object;
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


        enum keymap_format {
            keymap_format_no_keymap = 0, // no keymap; client must understand how to interpret the raw keycode
            keymap_format_xkb_v1 = 1 // libxkbcommon compatible; to determine the xkb keycode, clients must add 8 to the key event keycode
        };

        enum key_state {
            key_state_released = 0, // key is not pressed
            key_state_pressed = 1 // key is pressed
        };

        void send_keymap(uint32_t format, int32_t fd, uint32_t size);
        void send_keymap(struct ::wl_resource *resource, uint32_t format, int32_t fd, uint32_t size);
        void send_enter(uint32_t serial, struct ::wl_resource *surface, const QByteArray &keys);
        void send_enter(struct ::wl_resource *resource, uint32_t serial, struct ::wl_resource *surface, const QByteArray &keys);
        void send_leave(uint32_t serial, struct ::wl_resource *surface);
        void send_leave(struct ::wl_resource *resource, uint32_t serial, struct ::wl_resource *surface);
        void send_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
        void send_key(struct ::wl_resource *resource, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
        void send_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
        void send_modifiers(struct ::wl_resource *resource, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
        void send_repeat_info(int32_t rate, int32_t delay);
        void send_repeat_info(struct ::wl_resource *resource, int32_t rate, int32_t delay);

    protected:
        virtual Resource *keyboard_allocate();

        virtual void keyboard_bind_resource(Resource *resource);
        virtual void keyboard_destroy_resource(Resource *resource);

        virtual void keyboard_release(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_keyboard_interface m_wl_keyboard_interface;

        static void handle_release(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_touch
    {
    public:
        wl_touch(struct ::wl_client *client, int id, int version);
        wl_touch(struct ::wl_display *display, int version);
        wl_touch(struct ::wl_resource *resource);
        wl_touch();

        virtual ~wl_touch();

        class Resource
        {
        public:
            Resource() : touch_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_touch *touch_object;
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


        void send_down(uint32_t serial, uint32_t time, struct ::wl_resource *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
        void send_down(struct ::wl_resource *resource, uint32_t serial, uint32_t time, struct ::wl_resource *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
        void send_up(uint32_t serial, uint32_t time, int32_t id);
        void send_up(struct ::wl_resource *resource, uint32_t serial, uint32_t time, int32_t id);
        void send_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
        void send_motion(struct ::wl_resource *resource, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
        void send_frame();
        void send_frame(struct ::wl_resource *resource);
        void send_cancel();
        void send_cancel(struct ::wl_resource *resource);

    protected:
        virtual Resource *touch_allocate();

        virtual void touch_bind_resource(Resource *resource);
        virtual void touch_destroy_resource(Resource *resource);

        virtual void touch_release(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_touch_interface m_wl_touch_interface;

        static void handle_release(
            ::wl_client *client,
            struct wl_resource *resource);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_output
    {
    public:
        wl_output(struct ::wl_client *client, int id, int version);
        wl_output(struct ::wl_display *display, int version);
        wl_output(struct ::wl_resource *resource);
        wl_output();

        virtual ~wl_output();

        class Resource
        {
        public:
            Resource() : output_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_output *output_object;
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


        enum subpixel {
            subpixel_unknown = 0,
            subpixel_none = 1,
            subpixel_horizontal_rgb = 2,
            subpixel_horizontal_bgr = 3,
            subpixel_vertical_rgb = 4,
            subpixel_vertical_bgr = 5
        };

        enum transform {
            transform_normal = 0,
            transform_90 = 1,
            transform_180 = 2,
            transform_270 = 3,
            transform_flipped = 4,
            transform_flipped_90 = 5,
            transform_flipped_180 = 6,
            transform_flipped_270 = 7
        };

        enum mode {
            mode_current = 0x1, // indicates this is the current mode
            mode_preferred = 0x2 // indicates this is the preferred mode
        };

        void send_geometry(int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const QString &make, const QString &model, int32_t transform);
        void send_geometry(struct ::wl_resource *resource, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const QString &make, const QString &model, int32_t transform);
        void send_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        void send_mode(struct ::wl_resource *resource, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        void send_done();
        void send_done(struct ::wl_resource *resource);
        void send_scale(int32_t factor);
        void send_scale(struct ::wl_resource *resource, int32_t factor);

    protected:
        virtual Resource *output_allocate();

        virtual void output_bind_resource(Resource *resource);
        virtual void output_destroy_resource(Resource *resource);

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

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_region
    {
    public:
        wl_region(struct ::wl_client *client, int id, int version);
        wl_region(struct ::wl_display *display, int version);
        wl_region(struct ::wl_resource *resource);
        wl_region();

        virtual ~wl_region();

        class Resource
        {
        public:
            Resource() : region_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_region *region_object;
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
        virtual Resource *region_allocate();

        virtual void region_bind_resource(Resource *resource);
        virtual void region_destroy_resource(Resource *resource);

        virtual void region_destroy(Resource *resource);
        virtual void region_add(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
        virtual void region_subtract(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_region_interface m_wl_region_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_add(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);
        static void handle_subtract(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_subcompositor
    {
    public:
        wl_subcompositor(struct ::wl_client *client, int id, int version);
        wl_subcompositor(struct ::wl_display *display, int version);
        wl_subcompositor(struct ::wl_resource *resource);
        wl_subcompositor();

        virtual ~wl_subcompositor();

        class Resource
        {
        public:
            Resource() : subcompositor_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_subcompositor *subcompositor_object;
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


        enum error {
            error_bad_surface = 0 // the to-be sub-surface is invalid
        };

    protected:
        virtual Resource *subcompositor_allocate();

        virtual void subcompositor_bind_resource(Resource *resource);
        virtual void subcompositor_destroy_resource(Resource *resource);

        virtual void subcompositor_destroy(Resource *resource);
        virtual void subcompositor_get_subsurface(Resource *resource, uint32_t id, struct ::wl_resource *surface, struct ::wl_resource *parent);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_subcompositor_interface m_wl_subcompositor_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_get_subsurface(
            ::wl_client *client,
            struct wl_resource *resource,
            uint32_t id,
            struct ::wl_resource *surface,
            struct ::wl_resource *parent);

        QMultiMap<struct ::wl_client*, Resource*> m_resource_map;
        Resource *m_resource;
        struct ::wl_global *m_global;
        uint32_t m_globalVersion;
    };

    class Q_WAYLAND_SERVER_WAYLAND_EXPORT wl_subsurface
    {
    public:
        wl_subsurface(struct ::wl_client *client, int id, int version);
        wl_subsurface(struct ::wl_display *display, int version);
        wl_subsurface(struct ::wl_resource *resource);
        wl_subsurface();

        virtual ~wl_subsurface();

        class Resource
        {
        public:
            Resource() : subsurface_object(0), handle(0) {}
            virtual ~Resource() {}

            wl_subsurface *subsurface_object;
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


        enum error {
            error_bad_surface = 0 // wl_surface is not a sibling or the parent
        };

    protected:
        virtual Resource *subsurface_allocate();

        virtual void subsurface_bind_resource(Resource *resource);
        virtual void subsurface_destroy_resource(Resource *resource);

        virtual void subsurface_destroy(Resource *resource);
        virtual void subsurface_set_position(Resource *resource, int32_t x, int32_t y);
        virtual void subsurface_place_above(Resource *resource, struct ::wl_resource *sibling);
        virtual void subsurface_place_below(Resource *resource, struct ::wl_resource *sibling);
        virtual void subsurface_set_sync(Resource *resource);
        virtual void subsurface_set_desync(Resource *resource);

    private:
        static void bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id);
        static void destroy_func(struct ::wl_resource *client_resource);

        Resource *bind(struct ::wl_client *client, uint32_t id, int version);
        Resource *bind(struct ::wl_resource *handle);

        static const struct ::wl_subsurface_interface m_wl_subsurface_interface;

        static void handle_destroy(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_position(
            ::wl_client *client,
            struct wl_resource *resource,
            int32_t x,
            int32_t y);
        static void handle_place_above(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *sibling);
        static void handle_place_below(
            ::wl_client *client,
            struct wl_resource *resource,
            struct ::wl_resource *sibling);
        static void handle_set_sync(
            ::wl_client *client,
            struct wl_resource *resource);
        static void handle_set_desync(
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

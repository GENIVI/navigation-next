#ifndef QT_WAYLAND_QTKEY_EXTENSION
#define QT_WAYLAND_QTKEY_EXTENSION

#include <QtWaylandClient/private/wayland-qtkey-extension-client-protocol.h>
#include <QByteArray>
#include <QString>

QT_BEGIN_NAMESPACE

#if !defined(Q_WAYLAND_CLIENT_QTKEY_EXTENSION_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_CLIENT_QTKEY_EXTENSION_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_CLIENT_QTKEY_EXTENSION_EXPORT
#  endif
#endif

namespace QtWayland {
    class Q_WAYLAND_CLIENT_QTKEY_EXTENSION_EXPORT qt_key_extension
    {
    public:
        qt_key_extension(struct ::wl_registry *registry, int id, int version);
        qt_key_extension(struct ::qt_key_extension *object);
        qt_key_extension();

        virtual ~qt_key_extension();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::qt_key_extension *object);

        struct ::qt_key_extension *object() { return m_qt_key_extension; }
        const struct ::qt_key_extension *object() const { return m_qt_key_extension; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void dummy();

    protected:
        virtual void key_extension_qtkey(struct ::wl_surface *surface, uint32_t time, uint32_t type, uint32_t key, uint32_t modifiers, uint32_t nativeScanCode, uint32_t nativeVirtualKey, uint32_t nativeModifiers, const QString &text, uint32_t autorepeat, uint32_t count);

    private:
        void init_listener();
        static const struct qt_key_extension_listener m_qt_key_extension_listener;
        static void handle_qtkey(
            void *data,
            struct ::qt_key_extension *object,
            struct ::wl_surface *surface,
            uint32_t time,
            uint32_t type,
            uint32_t key,
            uint32_t modifiers,
            uint32_t nativeScanCode,
            uint32_t nativeVirtualKey,
            uint32_t nativeModifiers,
            const char *text,
            uint32_t autorepeat,
            uint32_t count);
        struct ::qt_key_extension *m_qt_key_extension;
    };
}

QT_END_NAMESPACE

#endif

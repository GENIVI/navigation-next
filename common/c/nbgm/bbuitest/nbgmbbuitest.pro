APP_NAME = nbgmbbuitest

CONFIG += qt warn_on cascades10

INCLUDEPATH +=  \
                ../../../nbpal/feature_130725_map3d_main/include \
                ../../../nbpal/feature_130725_map3d_main/include/qt_qnx \
                ../nbgmmain/include \
                ../nbgmmain/include/qt_qnx \
                ../nbgmmain/include/protected \
                ../nbgmmain/include/private \
                ../nbre/include \
                ../nbre/include/protected \
                ../nbre/include/private \
                ../rendersystem/gles/include \
                ../rendersystem/gles2/include \
                ./include

LIBS        +=  -lscreen -lEGL -lGLESv1_CM -lGLESv2 -lpng -lfreetype -lslog2 -liType -lWTLE -lgestures\
                -L../../projects/qt_qnx/SharedLibraries/nbgm/arm/so-le-v7 -lnbgmS \ 
                -L../libs/lib-arm -lpalbase -lpal_graphics -lpal_uitaskqueue -ljpeg_turbo -lbbcascadespickers

include(config.pri)

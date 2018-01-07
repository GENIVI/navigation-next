
######### Environment setting ############
LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)

######### Project setting ############
QT       += gui
INCLUDEPATH += ../include/data
INCLUDEPATH += ../include/private
INCLUDEPATH += ../include/signal
INCLUDEPATH += ../include/private/widgets
INCLUDEPATH += ../include/private/widgets/laneguidance
INCLUDEPATH += ../include/private/widgets/common
INCLUDEPATH += ../include/private/widgets/navlist
INCLUDEPATH += ../include/private/widgets/minimap
INCLUDEPATH += ../include/private/widgets/sar
INCLUDEPATH += ../include/private/widgets/routeselectbubble
INCLUDEPATH += ../include/private/widgets/laneguidance
INCLUDEPATH += ../include
linux{
INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/include/freetype2
}



SOURCES += \
    ../src/typetranslater.cpp \
    ../src/mapinterfacewrapper.cpp \
    ../src/navigationuikitimpl.cpp \
    ../src/navigationuikitwrapper.cpp\
    ../src/locationproviderlistener.cpp \
    ../src/localizedstringhelperimpl.cpp \
    ../src/navuipreferences.cpp \
    ../src/navuirouteoptions.cpp \
    ../src/widgets/widgetbase.cpp \
    ../src/widgets/widgetmanager.cpp \
    ../src/widgets/announcementimpl.cpp \
    ../src/widgets/nogpswidgetimpl.cpp \
    ../src/widgets/currentroadimpl.cpp \
    ../src/widgets/stackmaneuverwidgetimpl.cpp \
    ../src/widgets/speedlimitwidgetimpl.cpp \
    ../src/widgets/navigationheaderwidgetimpl.cpp \
    ../src/widgets/navigationfooterwidgetimpl.cpp \
    ../src/widgets/common/imageprovider.cpp \
    ../src/widgets/overflowmenuwidgetimpl.cpp \
    ../src/widgets/arrivalfooterwidgetimpl.cpp \
    ../src/widgets/arrivalheaderwidgetimpl.cpp \
    ../src/widgets/detourfooterimpl.cpp \
    ../src/widgets/detourheaderwidgetimpl.cpp \
    ../src/widgets/enhancednavstartupnotifyimpl.cpp \
    ../src/widgets/enhancedstartheaderwidgetimpl.cpp \
    ../src/widgets/pinbubblelayerimpl.cpp \
    ../src/widgets/recalconcalltextwidgetimpl.cpp \
    ../src/widgets/retrywidgetimpl.cpp \
    ../src/widgets/routerequestfooterimpl.cpp \
    ../src/widgets/routeselectfooterimpl.cpp \
    ../src/widgets/routeselectheaderwidgetimpl.cpp \
    ../src/widgets/routeselectlistwidgetimpl.cpp \
    ../src/widgets/routeselectstatusbarimpl.cpp \
    ../src/widgets/startingnavfooterimpl.cpp \
    ../src/widgets/statusbarimpl.cpp \
    ../src/widgets/stopnavigationwidgetimpl.cpp \
    ../src/widgets/tripoverviewimpl.cpp \
    ../src/widgets/endtripstatusbarimpl.cpp \
    ../src/widgets/footerwidgetbase.cpp \
    ../src/widgets/navlist/navigationlistwidgetimpl.cpp \
    ../src/widgets/navlist/maneuverdataobject.cpp \
    ../src/widgets/trafficaheadwidgetimpl.cpp \
    ../src/widgets/overviewfooterimpl.cpp \
    ../src/widgets/minimap/minimaprtsimpl.cpp \
    ../src/widgets/minimap/minimaptripoverviewimpl.cpp \
    ../src/widgets/minimap/minimapwidgetimpl.cpp \
    ../src/widgets/minimap/minimapbutton.cpp \
    ../src/widgets/sar/sarwidgetimpl.cpp \
    ../src/widgets/sar/sarwidgetinternal.cpp \
    ../src/widgets/routeselectbubble/routeselectbubblelayerimpl.cpp \
    ../src/widgets/routeselectbubble/routeselectbubblelayouthelper.cpp \
    ../src/widgets/routeselectbubble/routeselectbubbleInternal.cpp \
    ../src/widgets/navigationerrormessagebox.cpp \
    ../src/widgets/laneguidance/laneguidancewidgetinternal.cpp \
    ../src/widgets/laneguidance/laneguidancewidgetimpl.cpp \
    ../src/widgets/common/imagewidget.cpp \
    ../src/widgets/laneguidanceimageprovider.cpp

HEADERS  += \
    ../include/navigationuikit.h \
    ../include/navigationuimapinterface.h \
    ../include/signal/navigationuisignals.h \
    ../include/data/nkuitypes.h \
    ../include/data/userpreferences.h \
    ../include/private/localizedstringhelperimpl.h \
    ../include/private/mapinterfacewrapper.h \
    ../include/private/widgets/widgetbase.h \
    ../include/private/widgets/widgetmanager.h \
    ../include/private/widgets/announcementimpl.h \
    ../include/private/widgets/nogpswidgetimpl.h \
    ../include/private/widgets/currentroadimpl.h \
    ../include/private/widgets/stackmaneuverwidgetimpl.h \
    ../include/private/widgets/speedlimitwidgetimpl.h \
    ../include/private/widgets/navigationheaderwidgetimpl.h \
    ../include/private/widgets/navigationfooterwidgetimpl.h \
    ../include/private/widgets/common/imageprovider.h \
    ../include/private/widgets/arrivalheaderwidgetimpl.h \
    ../include/private/widgets/detourfooterimpl.h \
    ../include/private/widgets/detourheaderwidgetimpl.h \
    ../include/private/widgets/endtripstatusbarimpl.h \
    ../include/private/widgets/enhancednavstartupnotifyimpl.h \
    ../include/private/widgets/enhancedstartheaderwidgetimpl.h \
    ../include/private/widgets/footerwidgetbase.h \
    ../include/private/widgets/overflowmenuwidgetimpl.h \
    ../include/private/widgets/pinbubblelayerimpl.h \
    ../include/private/widgets/recalconcalltextwidgetimpl.h \
    ../include/private/widgets/retrywidgetimpl.h \
    ../include/private/widgets/routerequestfooterimpl.h \
    ../include/private/widgets/routeselectfooterimpl.h \
    ../include/private/widgets/routeselectheaderwidgetimpl.h \
    ../include/private/widgets/routeselectlistwidgetimpl.h \
    ../include/private/widgets/routeselectstatusbarimpl.h \
    ../include/private/widgets/startingnavfooterimpl.h \
    ../include/private/widgets/statusbarimpl.h \
    ../include/private/widgets/stopnavigationwidgetimpl.h \
    ../include/private/widgets/tripoverviewimpl.h \
    ../include/private/widgets/arrivalfooterwidgetimpl.h \
    ../include/private/widgets/navlist/navigationlistwidgetimpl.h \
    ../include/private/widgets/navlist/maneuverdataobject.h \
    ../include/private/widgets/trafficaheadwidgetimpl.h \
    ../include/private/widgets/overviewfooterimpl.h \
    ../include/private/widgets/minimap/minimaprtsimpl.h \
    ../include/private/widgets/minimap/minimaptripoverviewimpl.h \
    ../include/private/widgets/minimap/minimapwidgetimpl.h \
    ../include/private/widgets/minimap/minimapbutton.h \
    ../include/private/widgets/sar/sarwidgetimpl.h \
    ../include/private/widgets/sar/sarwidgetinternal.h \
    ../include/private/widgets/routeselectbubble/routeselectbubblelayerimpl.h \
    ../include/private/widgets/routeselectbubble/routeselectbubblelayouthelper.h \
    ../include/private/widgets/routeselectbubble/routeselectbubbleInternal.h \
    ../include/private/widgets/navigationerrormessagebox.h \
    ../include/private/widgets/laneguidance/laneguidancewidgetimpl.h \
    ../include/private/widgets/laneguidance/laneguidancewidgetinternal.h \
    ../include/private/typetranslater.h \
    ../include/private/navigationuikitwrapper.h \
    ../include/private/navigationuikitimpl.h \
    ../include/private/locationproviderlistener.h \
    ../include/private/widgets/common/imagewidget.h \
    ../include/private/widgets/laneguidanceimageprovider.h

OTHER_FILES += \
    ../resource/qml/Annoucement.qml \
    ../resource/qml/CurrentRoad.qml \
    ../resource/qml/Footer.qml \
    ../resource/qml/GpsStatus.qml \
    ../resource/qml/Header.qml \
    ../resource/qml/MiniMap.qml \
    ../resource/qml/NavList.qml \
    ../resource/qml/NextTurn.qml \
    ../resource/qml/OverFlow.qml \
    ../resource/qml/SpeedLimit.qml \
    ../resource/WidgetLayoutConfig.xml \
    ../resource/images/icon_audio_high.png \
    ../resource/images/icon_audio_low.png \
    ../resource/images/icon_audio_off.png \
    ../resource/font/LG_NexGen.ttf \
    ../resource/qml/RouteRequestStatus.qml \
    ../resource/qml/TrafficAhead.qml\
    ../resource/qml/RouteSelectStatusBar.qml\
    ../resource/qml/ManeuverList.qml \
    ../resource/qml/RouteSelectList.qml \
    ../resource/qml/RouteSelectHeader.qml \
    ../resource/qml/Retry.qml \
    ../resource/qml/ArrivalHeader.qml \
    ../resource/images/all_routeoverview.png \
    ../resource/images/back_to_nav.png \
    ../resource/images/bubble_left_up_off.png \
    ../resource/images/bubble_left_up_on.png \
    ../resource/images/bubble_right_up_off.png \
    ../resource/images/bubble_right_up_on.png \
    ../resource/images/close_x.png \
    ../resource/images/down_icon.png \
    ../resource/images/minimap_traffic_congestion_orange.png \
    ../resource/images/minimap_traffic_congestion_red.png \
    ../resource/images/minimap_traffic_congestion_yellow.png \
    ../resource/images/minimap_traffic_incident_orange.png \
    ../resource/images/minimap_traffic_incident_red.png \
    ../resource/images/minimap_traffic_incident_yellow.png \
    ../resource/images/more_white.png \
    ../resource/images/routeoverview.png \
    ../resource/images/speed_limit.png \
    ../resource/images/traffic_delay_gray.png \
    ../resource/images/traffic_delay_green.png \
    ../resource/images/traffic_delay_red.png \
    ../resource/images/traffic_delay_yellow.png \
    ../resource/images/laneguidance_left.png \
    ../resource/images/laneguidance_right.png \
    ../resource/images/laneguidance_strip.png \
    ../resource/qml/Style.qml \
    ../resource/qml/EndTrip.qml \
    ../resource/qml/ErrorMessageBox.qml

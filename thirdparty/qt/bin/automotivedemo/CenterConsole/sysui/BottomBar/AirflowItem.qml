import QtQuick 2.0

import controls 1.0
import service.climate 1.0

Symbol {
    id: symbol
    property string icon: ""
    property var climateId: ""
    scale: 0.7
    name: icon
    active: ClimateService.airflow.value === climateId
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (active)
                ClimateService.airflow.value = 0
            else
                ClimateService.airflow.value = climateId
        }
    }
    onActiveChanged: {
        if (active === true)
            symbol.fanDir = name
    }
}

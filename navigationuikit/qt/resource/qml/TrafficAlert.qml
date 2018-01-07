import QtQuick 1.1

Item{
    id: traffic
    width: 116/*overviewImage.width > trafficImage.width ? overviewImage.width : trafficImage.width*/
    height: 50/*overviewImage.height + trafficImage.height*/

    Image {
        id: trafficImage
        objectName: "trafficAlert"
        source: "../images/minimap_traffic_incident_red.png";
        anchors{
            left: parent.left
            top: parent.top
        }
    }
}

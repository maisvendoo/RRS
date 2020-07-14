import QtQuick 2.0
import QtQuick.Controls 2.0
import "BaseElements" as BaseElements
ApplicationWindow
{
    id: applicationWindow
    visible: true
    title: "Russian Railway Simulator"
    minimumWidth: 400
    minimumHeight: 150
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight
    modality: Qt.ApplicationModal

    BaseElements.Constants{id: constants; anchors.fill: parent}
    Text {
        anchors.fill: parent
        text: qsTr("Application already running")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: constants.grey
        font{
            pixelSize: constants.lTextSize * 1.5
            bold: true
        }
    }
}

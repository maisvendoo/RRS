import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../" as Main

Item {
    id: loader
    property var sourceComponent
    Loader{
        anchors.fill: parent
        asynchronous: true
        sourceComponent: Item{
            property string name: loader.sourceComponent.name
            property real textScale: 0.55 /// высота текста относительно ячейки в %
            RowLayout{
                anchors{
                    fill: parent
                    leftMargin: constants.hSpacing * 0.2
                    rightMargin: parent.width * 0.08
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    implicitWidth: 12
                    Text{
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        text: name
                        color: constants.grey
                        font.pixelSize: parent.height * textScale
                        wrapMode: Text.WordWrap
                        maximumLineCount: 1
                    }
                }
            }
        }
    }
}

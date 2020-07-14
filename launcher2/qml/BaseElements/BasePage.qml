import QtQuick 2.0

Item {
    id: basePage

    /// Данные для базовой страницы
    property alias leftData: leftItem.data
    //property alias rightData: rightItem.data
    property alias centerData: centerItem.data

    /// Настройка пропорций базовой страницы
    property real leftItemWidth: width * 0.4
    property real coefMargin: 5

    MouseArea{anchors.fill: parent}

    Item{
        id: body
        anchors.fill: parent

        Item{
            id: leftItem
            anchors.left: parent.left
            height: parent.height
            width: leftItemWidth
        }

        Item{
            id: centerItem
            anchors.left: leftItem.right
            height: parent.height
            anchors.right: parent.right
        }
    }
}

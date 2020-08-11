import QtQuick 2.0
import QtQuick.Window 2.3

Rectangle{
    property string imagePath
    color: "#30000000"

    Image {
        id: image
        width: parent.width * 0.6
        height: parent.height * 0.6
        anchors.horizontalCenter: parent.horizontalCenter
        y: -height
        source: imagePath
        fillMode: Image.PreserveAspectFit
    }

    PropertyAnimation {
        target: image
        running: true
        property: "y"
        from: image.y
        to: parent.height / 2 - (image.height / 2)
        easing.type: Easing.InOutQuad; duration: 150
    }
}

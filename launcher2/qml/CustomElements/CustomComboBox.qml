import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox{
    id: control
    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: control
            onPressedChanged: canvas.requestPaint()
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fill();
        }
    }

    contentItem: Text {
            leftPadding: 0
            rightPadding: control.indicator.width + control.spacing
            text: control.displayText
            font: control.font
            color: control.pressed ? constants.lightgrey : constants.grey
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
}

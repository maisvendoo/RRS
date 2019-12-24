import QtQuick 2.0
import QtGraphicalEffects 1.0

Item
{
    id: lightButton
    property string text
    property string image
    property color  textColor: "white"
    property color  color: constants.green
    property real   radius: height * 0.05
    property color  disabledColor: constants.lightgrey
    property real   textSize: width * 0.5
    property real   imageSize: height * 0.65
    property real   hSpacing: constants.hSpacing
    property real   vSpacing: constants.vSpacing
    property bool   isCenter: false
    property int    hTextAlign: Text.AlignHCenter
    property int    hImageAlign: Image.AlignRight
    property int    fontWeight: Font.Bold
    property int    upperCase: Font.AllUppercase

    property real effectWidth: lightButton.width * 0.005
    signal clicked()
    signal pressAndHold()
    signal press()
    signal releas()
    Rectangle{
        anchors.fill: parent
        color: enabled ? lightButton.color : disabledColor
        radius: lightButton.radius
        layer.enabled: true
        layer.effect: DropShadow{
            color: constants.grey
            horizontalOffset: 1
            verticalOffset: 5
            radius: 3
            samples: 7
            spread: 0
        }

        Component{
            id: topBorder
            Rectangle{
                anchors.fill: parent
                radius: lightButton.radius
                color: "white"
                opacity: 0.3
            }
        }

        Component{
            id: bottomBorder
            Rectangle{
                radius: lightButton.radius
                color: "black"
                opacity: 0.2
            }
        }

        Loader{id: dataLoader; anchors{fill: parent;leftMargin: hSpacing; rightMargin: hSpacing; topMargin: vSpacing; bottomMargin: vSpacing} sourceComponent: d.getComponent()}

        Component{
            id: comboComponent
            Item{
                anchors.fill: parent
                Text {
                    height: parent.height
                    width: parent.width - imageRow.width
                    text: lightButton.text
                    color: textColor
                    font.pixelSize: textSize
                    font.capitalization: upperCase
                    font.weight: fontWeight
                    maximumLineCount: 3
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: hTextAlign
                }
                Image{
                    id: imageRow
                    sourceSize.height: lightButton.imageSize
                    source: lightButton.image
                    anchors{
                        verticalCenter: parent.verticalCenter
                        left: hImageAlign === Image.AlignLeft ? parent.left : undefined
                        right: hImageAlign === Image.AlignRight ? parent.right : undefined
                        centerIn: isCenter ? parent : undefined
                    }
                }
            }
        }
        Component{
            id: imageComponent
            Item{
                anchors.fill: parent
                Image{
                    sourceSize.height: lightButton.imageSize
                    source: lightButton.image
                    anchors{
                        verticalCenter: parent.verticalCenter
                        left: hImageAlign === Image.AlignLeft ? parent.left : undefined
                        right: hImageAlign === Image.AlignRight ? parent.right : undefined
                        centerIn: isCenter ? parent : undefined
                    }
                }
            }
        }
        Component{
            id: textComponent
            Text {
                anchors.fill: parent
                text: lightButton.text
                color: textColor
                font.pixelSize: textSize
                font.capitalization: upperCase
                font.weight: fontWeight
                maximumLineCount: 3
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: hTextAlign
            }
        }

        Loader{id: overlayLoader; anchors.fill: parent}
        Component{id: overlayArea; Rectangle{anchors.fill: parent; radius: lightButton.radius; color: "black"; opacity: 0.25}}

        MouseArea{
            anchors.fill: parent
            enabled: lightButton.enabled
            onPressed: {overlayLoader.sourceComponent = overlayArea; radius = lightButton.radius * 1.3; lightButton.press()}
            onReleased: {overlayLoader.sourceComponent = undefined; radius = lightButton.radius / 1.3; lightButton.releas()}
            onClicked: {lightButton.clicked()}
            onPressAndHold: {lightButton.pressAndHold()}
        }

        QtObject{
            id: d
            function getComponent()
            {
                /// кнопка с текстом и картинкой
                if(lightButton.image.length && lightButton.text.length)
                    return comboComponent
                if(lightButton.image.length)
                    return imageComponent
                if(lightButton.text.length)
                    return textComponent
                return undefined
            }
        }
    }
}

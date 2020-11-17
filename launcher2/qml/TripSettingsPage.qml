import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import "BaseElements" as BaseElements
import "CustomElements" as CustomElements
import RoutesModel 1.0
import TrainsModel 1.0
import WaypointModel 1.0

BaseElements.BasePage {
    id: tripSettings
    function getMargins(){return tripSettings.height * 0.01}

    WaypointModel{id: waypoint}

    leftData: Item{
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.leftMargin: getMargins()
        anchors.rightMargin: getMargins()
        anchors.topMargin: getMargins()
        anchors.bottomMargin: getMargins()
        Item{
            anchors.top: parent.top
            width: parent.width
            height: (parent.height * 0.5) - (getMargins() * 0.5)
            ColumnLayout{
                anchors.fill: parent
                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    color: constants.grey
                    text: qsTr("Change route")
                }
                Rectangle{
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "white"
                    /// Model routes

                    ListView{
                        id: viewRoutes
                        anchors.fill: parent
                        model: RoutesModel{}
                        highlight: Rectangle {color: constants.lightgreen}
                        highlightMoveDuration: 200
                        clip: true
                        delegate: Item{
                            property variant currentLine: model
                            property int currentIndex: index
                            height: viewRoutes.height / 6
                            width: viewRoutes.width
                            MouseArea{
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    viewRoutes.currentIndex = index
                                    reload.reloadImage(parent.currentLine)
                                    reload.reloadDescriptionRoute(parent.currentLine)
                                }
                                onExited: {
                                    if(parent.currentLine.image !== String())
                                        loaderImage.sourceComponent = undefined
                                }
                            }
                            BaseElements.BaseDelegate{anchors.fill: parent; sourceComponent: currentLine}
                        }
                        onCurrentIndexChanged: {
                            var currentRoute = currentItem.currentLine.dir
                            simParameters.setRoute(currentRoute);
                            waypoint.setRoute(currentRoute);
                            station.model = waypoint
                        }
                    }
                }

                CustomElements.CustomComboBox{
                    id: station
                    textRole: 'name'
                    Layout.fillWidth: true
                    onCurrentTextChanged: {
                        reload.reloadOrdinate()
                        var currentItem = delegateModel.items.get(currentIndex)
                        simParameters.setWaypoint(currentItem.model.name)
                    }
                }
                RowLayout{
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        color: constants.grey
                        font.pixelSize: constants.sTextSize * 0.7
                        text: qsTr("Ordinate, m")
                    }
                    SpinBox{
                        id: ordinate
                        readonly property int decimals: 2
                        readonly property real realFrom: 0.0
                        readonly property real realTo: 9999999.0
                        readonly property real realStepSize: 0.01
                        readonly property real factor: Math.pow(10, decimals)
                        property real realValue: 0.0

                        Layout.fillWidth: true
                        stepSize: realStepSize*factor
                        value: realValue*factor
                        to : realTo*factor
                        from : realFrom*factor
                        validator: DoubleValidator {
                            bottom: Math.min(ordinate.from, ordinate.to)*ordinate.factor
                            top:  Math.max(ordinate.from, ordinate.to)*ordinate.factor
                        }

                        textFromValue: function(value, locale) {
                            return parseFloat(value*1.0/factor).toFixed(decimals);
                        }
                        onDisplayTextChanged: {simParameters.setOrdinate(parseFloat(value*1.0/factor).toFixed(decimals))}
                    }
                    CustomElements.CustomComboBox{
                        id: direction
                        Layout.fillWidth: true
                        textRole: "direction"
                        model: ListModel{
                            id: directionModel
                            property var currentIndex: index
                            ListElement{direction: qsTr("Forward"); isBackward: false}
                            ListElement{direction: qsTr("Backward"); isBackward: true}
                        }
                        onCurrentIndexChanged:
                        {
                            simParameters.setDirection(directionModel.get(currentIndex).isBackward)
                            reload.reloadOrdinate()
                        }
                    }
                }
            }
        }
        ColumnLayout{
            width: parent.width
            height: (parent.height * 0.5) - (getMargins() * 0.5)
            anchors.bottom: parent.bottom
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                color: constants.grey
                text: qsTr("Change train")
            }
            Rectangle{
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "white"
                /// Model train

                ListView{
                    id: viewTrains
                    anchors.fill: parent
                    model: TrainsModel{}
                    highlight: Rectangle {color: constants.lightgreen}
                    highlightMoveDuration: 200
                    clip: true
                    delegate: Item{
                        property variant currentLine: model
                        property int currentIndex: index
                        height: viewTrains.height / 9
                        width: viewTrains.width
                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                viewTrains.currentIndex = index
                                reload.reloadImage(parent.currentLine)
                                reload.reloadDescriptionTrain(parent.currentLine)
                            }
                            onExited: {
                                if(parent.currentLine.image !== String())
                                    loaderImage.sourceComponent = undefined
                            }
                        }
                        BaseElements.BaseDelegate{anchors.fill: parent; sourceComponent: currentLine}
                    }
                    onCurrentIndexChanged: simParameters.setTrain(currentItem.currentLine.dir)
                }
            }
        }
    }
    centerData: Item{
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.leftMargin: getMargins()
        anchors.rightMargin: getMargins()
        anchors.topMargin: getMargins()
        anchors.bottomMargin: getMargins()
        ColumnLayout{
            anchors.fill: parent
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                color: constants.grey
                text: qsTr("Route description")
            }
            Rectangle{
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "white"
                Text {
                    id: descriptionRoute
                    anchors.fill: parent
                    wrapMode: Text.WrapAnywhere
                    color: constants.grey
                }
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                color: constants.grey
                text: qsTr("Train description")
            }
            Rectangle{
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "white"
                Text {
                    id: descriptionTrain
                    anchors.fill: parent
                    wrapMode: Text.WrapAnywhere
                    color: constants.grey
                }
            }
        }
    }

    Loader{
        id: loaderImage
        property string imagePath
        anchors.fill: parent
        onLoaded: item.imagePath = imagePath
    }

    Component{
        id: imageComponent
        ImageComponent{}
    }

    QtObject{
        id: reload
        function reloadImage(model)
        {
            impl.reloadImage(model)
        }

        function reloadDescriptionTrain(model)
        {
            impl.reloadDescriptionTrain(model)
        }

        function reloadDescriptionRoute(model)
        {
            impl.reloadDescriptionRoute(model)
        }

        function reloadOrdinate()
        {
            impl.reloadOrdinate()
        }

        function reloadAll(model)
        {
            impl.reloadImage(model)
            impl.reloadDescriptionTrain(model)
            impl.reloadDescriptionRoute(model)
        }

        property QtObject impl: QtObject{
            function reloadImage(model)
            {
                if(model.image !== String()){
                    loaderImage.imagePath = model.image
                    loaderImage.sourceComponent = imageComponent
                }
            }
            function reloadDescriptionTrain(model)
            {
                if(model.description !== String())
                    descriptionTrain.text = model.description
            }
            function reloadDescriptionRoute(model)
            {
                if(model.description !== String())
                    descriptionRoute.text = model.description
            }
            function reloadOrdinate()
            {
                if(station.delegateModel)
                {
                    var currentItem = station.delegateModel.items.get(station.currentIndex)
                    if(!simParameters.getDirection())
                        ordinate.realValue = currentItem.model.forward
                    else
                        ordinate.realValue = currentItem.model.backward
                }
            }
        }
    }
}


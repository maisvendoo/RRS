import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "BaseElements" as BaseElements
ApplicationWindow
{
    id: applicationWindow
    visible: true
    title: "Russian Railway Simulator"
    minimumWidth: 1000
    minimumHeight: 600
    maximumHeight: minimumHeight
    maximumWidth: minimumWidth
    modality: Qt.ApplicationModal

    BaseElements.Constants{id: constants; anchors.fill: parent}

    header: TabBar{
        id: tabBar
        width: 320
        Repeater{
            model: mainData.tabBarModel()
            TabButton{
                text: modelData
                background:
                    Rectangle{
                    color: parent === tabBar.currentItem ? "white" : constants.lightgrey
                }
            }
        }
    }

    SwipeView{
        id: swipeView
        interactive: false
        width: parent.width
        height: parent.height * 0.9
        anchors.top: parent.top
        currentIndex: tabBar.currentIndex

        TripSettingsPage{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Item {
        width: parent.width
        height: parent.height - swipeView.height
        anchors.bottom: parent.bottom
        BaseElements.BaseButton{
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: parent.width * 0.008
            textSize: constants.mTextSize * 0.8
            width: parent.width * 0.1
            height: parent.height * 0.8
            text: qsTr("Start")
            onClicked: {simRun.startSimulator()}
        }
    }

    background: Rectangle{
        color: constants.lightgrey
    }

    function arWidth(width)
    {
        if(mainData.aspectRatio() > 1.33)    /// если не 4:3
            return width / mainData.aspectRatio() * 1.33
        return width
    }

    function imgPath(imageName){
        return 'qrc:/images/images/' + imageName
    }

    QtObject{
        id: mainData
        function tabBarModel()
        {
            var tabBarData = [qsTr("Ride settings"), qsTr("Graphics settings")]
            return tabBarData
        }

        function aspectRatio()
        {
            return applicationWindow.width / applicationWindow.height
        }
    }
}

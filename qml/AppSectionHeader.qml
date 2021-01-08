import QtQuick 2.0
import Sailfish.Silica 1.0

Row {
    id: thisItem

    property alias icon: image.source
    property alias name: label.text

    readonly property real maxWidth: (parent ? parent.width : Screen.width) - 2 * Theme.horizontalPageMargin

    spacing: Theme.paddingMedium
    anchors {
        right: parent.right
        rightMargin: Theme.horizontalPageMargin
    }

    Label {
        id: label

        anchors.verticalCenter: parent.verticalCenter
        height: Theme.itemSizeExtraSmall
        width: Math.min(implicitWidth, thisItem.maxWidth - image.width - thisItem.section)
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeSmall
        truncationMode: TruncationMode.Fade
        color: Theme.highlightColor
    }

    Image {
        id: image

        anchors.verticalCenter: parent.verticalCenter
        sourceSize: Qt.size(Theme.iconSizeSmall, Theme.iconSizeSmall)
        width: Theme.iconSizeSmall
        height: Theme.iconSizeSmall
        fillMode: Image.PreserveAspectFit
    }
}

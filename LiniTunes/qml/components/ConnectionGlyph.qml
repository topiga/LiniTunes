import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    id: root

    property string transport
    property color tintColor: "white"
    property int glyphSize: 12

    width: visible ? glyphSize : 0
    height: glyphSize

    Image {
        id: glyphImage
        anchors.fill: parent
        source: root.transport === "Wi-Fi" ? "/images/glyphs/wifi-connected.svg" : "/images/glyphs/ubs-connected.svg"
        sourceSize.width: root.glyphSize
        sourceSize.height: root.glyphSize
        fillMode: Image.PreserveAspectFit
        smooth: true
        opacity: 0
    }

    ColorOverlay {
        anchors.fill: glyphImage
        source: glyphImage
        color: root.tintColor
        visible: root.visible
    }
}

import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

ApplicationWindow {
    visible: true
    width: 1400
    height: 900
    title: typeof windowTitle !== 'undefined' ? windowTitle : "V2V Traffic Simulation"

    property bool showCircles: true

    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: ""
        }
        PluginParameter {
            name: "osm.useragent"
            value: "V2VTrafficSimulation/1.0"
        }
    }

    Map {
        id: map
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        plugin: mapPlugin
        center: QtPositioning.coordinate(mapCenterLat, mapCenterLon)
        zoomLevel: mapZoomLevel
        activeMapType: supportedMapTypes[0]

        property var startCentroid

        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
            }
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }

        WheelHandler {
            id: wheel
            rotationScale: 1/120
            property: "zoomLevel"
            onWheel: (event) => {
                const loc = map.toCoordinate(wheel.point.position)
                if (event.angleDelta.y > 0) {
                    map.zoomLevel = Math.min(map.zoomLevel + 0.5, 20)
                } else {
                    map.zoomLevel = Math.max(map.zoomLevel - 0.5, 2)
                }
                map.alignCoordinateToPoint(loc, wheel.point.position)
            }
        }

        MapItemView {
            id: interferenceEdgeView
            model: trafficSimulation.showInterferenceGraph ? trafficSimulation.interferenceGraph.edgeModel : null
            delegate: MapPolyline {
                line.width: 1
                line.color: {
                    if (strength > 0.7) return "#44FF44"
                    else if (strength > 0.4) return "#FFFF44"
                    else return "#FF4444"
                }
                path: [
                    QtPositioning.coordinate(lat1, lon1),
                    QtPositioning.coordinate(lat2, lon2)
                ]
                opacity: 0.6
            }
        }

        MapItemView {
            id: transmissionCircleView
            model: (trafficSimulation.showInterferenceGraph && showCircles) ? trafficSimulation.vehicleModel : null
            delegate: MapCircle {
                center: QtPositioning.coordinate(lat, lon)
                radius: transmissionRadius
                color: "transparent"
                border.width: 2
                border.color: "#6000AAFF"
            }
        }

        MapItemView {
            id: vehicleView
            model: trafficSimulation.vehicleModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(lat, lon)
                anchorPoint.x: carIcon.width / 2
                anchorPoint.y: carIcon.height / 2
                z: 1000

                sourceItem: Item {
                    id: carIcon
                    width: 24
                    height: 14

                    property color carColor: {
                        if (speed > 20) return "#00AAFF"
                        else if (speed > 10) return "#00DD00"
                        else if (speed > 0) return "#FFAA00"
                        else return "#FF0000"
                    }

                    Rectangle {
                        x: 2; y: 10
                        width: 5; height: 5; radius: 2.5
                        color: "#222"
                        Rectangle {
                            anchors.centerIn: parent
                            width: 2; height: 2; radius: 1
                            color: "#666"
                        }
                    }
                    Rectangle {
                        x: 17; y: 10
                        width: 5; height: 5; radius: 2.5
                        color: "#222"
                        Rectangle {
                            anchors.centerIn: parent
                            width: 2; height: 2; radius: 1
                            color: "#666"
                        }
                    }

                    Rectangle {
                        x: 0; y: 4
                        width: 24; height: 8
                        radius: 3
                        color: carIcon.carColor
                        border.color: Qt.darker(carIcon.carColor, 1.3)
                        border.width: 1
                    }

                    Rectangle {
                        x: 5; y: 1
                        width: 12; height: 5
                        radius: 2
                        color: Qt.darker(carIcon.carColor, 1.2)

                        Rectangle {
                            x: 1; y: 1; width: 4; height: 3; radius: 1
                            color: "#88CCFF"; opacity: 0.8
                        }
                        Rectangle {
                            x: 7; y: 1; width: 4; height: 3; radius: 1
                            color: "#88CCFF"; opacity: 0.8
                        }
                    }

                    Rectangle {
                        x: 21; y: 6; width: 3; height: 3; radius: 1
                        color: "#FFFF66"
                    }

                    Rectangle {
                        x: 0; y: 6; width: 2; height: 3; radius: 1
                        color: "#FF3333"
                    }
                }
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        spacing: 8
        z: 100

        Rectangle {
            width: 40; height: 40; radius: 8
            color: zoomInArea.containsMouse ? "#3a3a4a" : "#252535"
            border.color: "#4a4a5a"; border.width: 1
            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 20; font.bold: true; color: "#fff" }
            MouseArea {
                id: zoomInArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: map.zoomLevel = Math.min(map.zoomLevel + 1, 20)
            }
        }

        Rectangle {
            width: 40; height: 40; radius: 8
            color: zoomOutArea.containsMouse ? "#3a3a4a" : "#252535"
            border.color: "#4a4a5a"; border.width: 1
            Text { anchors.centerIn: parent; text: "-"; font.pixelSize: 20; font.bold: true; color: "#fff" }
            MouseArea {
                id: zoomOutArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: map.zoomLevel = Math.max(map.zoomLevel - 1, 2)
            }
        }

        Rectangle {
            width: 40; height: 40; radius: 8
            color: centerArea.containsMouse ? "#3a3a4a" : "#252535"
            border.color: "#4a4a5a"; border.width: 1
            Text { anchors.centerIn: parent; text: "◎"; font.pixelSize: 16; color: "#fff" }
            MouseArea {
                id: centerArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    map.center = QtPositioning.coordinate(mapCenterLat, mapCenterLon)
                    map.zoomLevel = mapZoomLevel
                }
            }
        }
    }

    Rectangle {
        id: sidebar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 280
        color: "#1a1a2e"

        Rectangle {
            id: sidebarHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 70
            color: "#16213e"

            Column {
                anchors.centerIn: parent
                spacing: 4

                Text {
                    text: "V2V TRAFFIC"
                    font.bold: true
                    font.pixelSize: 18
                    font.letterSpacing: 3
                    color: "#00d4ff"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "SIMULATION"
                    font.bold: true
                    font.pixelSize: 12
                    font.letterSpacing: 4
                    color: "#8a8aaa"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        ScrollView {
            anchors.top: sidebarHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            Column {
                width: 260
                spacing: 15

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        width: 115
                        height: 42
                        radius: 8
                        color: trafficSimulation.running ? "#e74c3c" : "#27ae60"
                        border.color: Qt.lighter(color, 1.2)
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: trafficSimulation.running ? "⏸ PAUSE" : "▶ START"
                            font.bold: true
                            font.pixelSize: 13
                            color: "#fff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: trafficSimulation.running ? trafficSimulation.pause() : trafficSimulation.start()
                        }
                    }

                    Rectangle {
                        width: 115
                        height: 42
                        radius: 8
                        color: "#34495e"
                        border.color: "#7f8c8d"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "↻ RESET"
                            font.bold: true
                            font.pixelSize: 13
                            color: "#fff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: trafficSimulation.reset()
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 95
                    radius: 10
                    color: "#16213e"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        Row {
                            spacing: 8
                            Text { text: "🚗"; font.pixelSize: 14 }
                            Text {
                                text: "VEHICLES"
                                font.bold: true
                                font.pixelSize: 11
                                font.letterSpacing: 1
                                color: "#8a8aaa"
                            }
                            Item { width: 60; height: 1 }
                            Text {
                                text: trafficSimulation.vehicleCount
                                font.pixelSize: 16
                                font.bold: true
                                color: "#00d4ff"
                            }
                        }

                        Row {
                            spacing: 8
                            Repeater {
                                model: [100, 500, 1000, 2000]
                                Rectangle {
                                    width: 50; height: 32; radius: 6
                                    color: trafficSimulation.vehicleCount === modelData ? "#00d4ff" : (vehicleBtnArea.containsMouse ? "#2c3e50" : "#1a1a2e")
                                    border.color: trafficSimulation.vehicleCount === modelData ? "#00d4ff" : "#3a3a4a"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData
                                        font.pixelSize: 11
                                        font.bold: true
                                        color: trafficSimulation.vehicleCount === modelData ? "#1a1a2e" : "#fff"
                                    }
                                    MouseArea {
                                        id: vehicleBtnArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: trafficSimulation.setVehicleCount(modelData)
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 125
                    radius: 10
                    color: "#16213e"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Row {
                            spacing: 8
                            Text { text: "⏱"; font.pixelSize: 14 }
                            Text {
                                text: "SIMULATION SPEED"
                                font.bold: true
                                font.pixelSize: 11
                                font.letterSpacing: 1
                                color: "#8a8aaa"
                            }
                        }

                        Text {
                            text: trafficSimulation.timeSpeed.toFixed(1) + "x"
                            font.pixelSize: 22
                            font.bold: true
                            color: "#ffa62b"
                        }

                        Slider {
                            id: speedSlider
                            width: parent.width
                            from: 0.1; to: 5.0; stepSize: 0.1
                            value: trafficSimulation.timeSpeed
                            onValueChanged: trafficSimulation.timeSpeed = value

                            background: Rectangle {
                                x: speedSlider.leftPadding
                                y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                                width: speedSlider.availableWidth
                                height: 4; radius: 2
                                color: "#2c3e50"
                                Rectangle {
                                    width: speedSlider.visualPosition * parent.width
                                    height: parent.height; radius: 2
                                    color: "#ffa62b"
                                }
                            }

                            handle: Rectangle {
                                x: speedSlider.leftPadding + speedSlider.visualPosition * (speedSlider.availableWidth - width)
                                y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                                width: 16; height: 16; radius: 8
                                color: speedSlider.pressed ? "#fff" : "#ffa62b"
                            }
                        }

                        Row {
                            spacing: 5
                            Repeater {
                                model: [0.5, 1.0, 2.0, 5.0]
                                Rectangle {
                                    width: 54; height: 26; radius: 6
                                    color: Math.abs(speedSlider.value - modelData) < 0.05 ? "#ffa62b" : "#2c3e50"
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData + "x"
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: Math.abs(speedSlider.value - modelData) < 0.05 ? "#1a1a2e" : "#aaa"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: trafficSimulation.timeSpeed = modelData
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 55
                    radius: 10
                    color: "#16213e"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        Text { text: "📡"; font.pixelSize: 16; anchors.verticalCenter: parent.verticalCenter }

                        Text {
                            text: "CONNECTIONS"
                            font.bold: true
                            font.pixelSize: 10
                            font.letterSpacing: 1
                            color: "#8a8aaa"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item { width: 20; height: 1 }

                        Rectangle {
                            width: 52; height: 28; radius: 14
                            color: trafficSimulation.showInterferenceGraph ? "#27ae60" : "#2c3e50"
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                width: 24; height: 24; radius: 12
                                color: "#fff"
                                x: trafficSimulation.showInterferenceGraph ? parent.width - width - 2 : 2
                                anchors.verticalCenter: parent.verticalCenter
                                Behavior on x { NumberAnimation { duration: 150 } }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: trafficSimulation.setShowInterferenceGraph(!trafficSimulation.showInterferenceGraph)
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 55
                    radius: 10
                    color: "#16213e"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 10

                        Text { text: "⭕"; font.pixelSize: 16; anchors.verticalCenter: parent.verticalCenter }

                        Text {
                            text: "RADIUS CIRCLES"
                            font.bold: true
                            font.pixelSize: 10
                            font.letterSpacing: 1
                            color: "#8a8aaa"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item { width: 20; height: 1 }

                        Rectangle {
                            width: 52; height: 28; radius: 14
                            color: showCircles ? "#3498db" : "#2c3e50"
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                width: 24; height: 24; radius: 12
                                color: "#fff"
                                x: showCircles ? parent.width - width - 2 : 2
                                anchors.verticalCenter: parent.verticalCenter
                                Behavior on x { NumberAnimation { duration: 150 } }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: showCircles = !showCircles
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 85
                    radius: 10
                    color: "#16213e"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Row {
                            spacing: 8
                            Text { text: "📊"; font.pixelSize: 14 }
                            Text {
                                text: "NETWORK STATS"
                                font.bold: true
                                font.pixelSize: 11
                                font.letterSpacing: 1
                                color: "#8a8aaa"
                            }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Nodes:"; font.pixelSize: 12; color: "#888" }
                            Text { text: roadGraphModel.nodeCount.toString(); font.pixelSize: 12; font.bold: true; color: "#00d4ff" }
                            Item { width: 15; height: 1 }
                            Text { text: "Edges:"; font.pixelSize: 12; color: "#888" }
                            Text { text: roadGraphModel.edgeCount.toString(); font.pixelSize: 12; font.bold: true; color: "#00d4ff" }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 130
                    radius: 10
                    color: "#16213e"
                    border.color: "#27ae60"
                    border.width: trafficSimulation.showInterferenceGraph ? 1 : 0
                    visible: trafficSimulation.showInterferenceGraph

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 5

                        Row {
                            spacing: 8
                            Text { text: "📡"; font.pixelSize: 12 }
                            Text {
                                text: "INTERFERENCE STATS"
                                font.bold: true
                                font.pixelSize: 10
                                font.letterSpacing: 1
                                color: "#8a8aaa"
                            }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Vehicles:"; font.pixelSize: 11; color: "#888" }
                            Text { text: trafficSimulation.interferenceGraph.nodeCount; font.pixelSize: 11; font.bold: true; color: "#27ae60" }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Connections:"; font.pixelSize: 11; color: "#888" }
                            Text { text: trafficSimulation.interferenceGraph.edgeCount; font.pixelSize: 11; font.bold: true; color: "#27ae60" }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Avg Degree:"; font.pixelSize: 11; color: "#888" }
                            Text { text: trafficSimulation.interferenceGraph.averageDegree.toFixed(2); font.pixelSize: 11; font.bold: true; color: "#ffa62b" }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Max Degree:"; font.pixelSize: 11; color: "#888" }
                            Text { text: trafficSimulation.interferenceGraph.maxDegree; font.pixelSize: 11; font.bold: true; color: "#e74c3c" }
                        }

                        Row {
                            spacing: 8
                            Text { text: "Radius:"; font.pixelSize: 11; color: "#888" }
                            Text { text: "100-500m"; font.pixelSize: 11; font.bold: true; color: "#00d4ff" }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 110
                    radius: 10
                    color: "#16213e"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Row {
                            spacing: 8
                            Text { text: "🚦"; font.pixelSize: 14 }
                            Text {
                                text: "SPEED LEGEND"
                                font.bold: true
                                font.pixelSize: 11
                                font.letterSpacing: 1
                                color: "#8a8aaa"
                            }
                        }

                        Row {
                            spacing: 8
                            Rectangle { width: 18; height: 10; radius: 3; color: "#00AAFF" }
                            Text { text: "> 70 km/h (Fast)"; font.pixelSize: 10; color: "#aaa" }
                        }
                        Row {
                            spacing: 8
                            Rectangle { width: 18; height: 10; radius: 3; color: "#00DD00" }
                            Text { text: "35-70 km/h"; font.pixelSize: 10; color: "#aaa" }
                        }
                        Row {
                            spacing: 8
                            Rectangle { width: 18; height: 10; radius: 3; color: "#FFAA00" }
                            Text { text: "< 35 km/h (Slow)"; font.pixelSize: 10; color: "#aaa" }
                        }
                        Row {
                            spacing: 8
                            Rectangle { width: 18; height: 10; radius: 3; color: "#FF0000" }
                            Text { text: "Stopped"; font.pixelSize: 10; color: "#aaa" }
                        }
                    }
                }

                Item { width: 1; height: 20 }
            }
        }
    }
}

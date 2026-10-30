import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Pane {
    id: panel
    visible: false
    width: 440
    height: Math.min(760, parent.height - 40)
    padding: 14
    z: 1500
    property url imageSource
    property var pendingPixel: null
    property string savedPath: ""
    property alias pointModel: references
    readonly property bool waitingForMap: pendingPixel !== null

    function addCoordinate(coordinate) {
        if (!waitingForMap || !coordinate.isValid) return
        references.append({pixelX: pendingPixel.x, pixelY: pendingPixel.y,
                              gpsLat: coordinate.latitude, gpsLon: coordinate.longitude})
        pendingPixel = null
        savedPath = ""
    }

    ListModel { id: references }
    background: Rectangle { color: "#f6f7fb"; radius: 10; border.color: "#64748b" }

    FileDialog {
        id: imageDialog
        title: "Image extraite de la caméra fixe"
        nameFilters: ["Image PNG (*.png)", "Images (*.jpg *.jpeg)"]
        onAccepted: {
            panel.imageSource = selectedFile
            references.clear()
            panel.pendingPixel = null
            panel.savedPath = ""
        }
    }
    FileDialog {
        id: saveDialog
        title: "Enregistrer les repères de calibration"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: ["Calibration JSON (*.json)"]
        onAccepted: {
            const points = []
            for (let i = 0; i < references.count; ++i) {
                const p = references.get(i)
                points.push({x:p.pixelX, y:p.pixelY, lat:p.gpsLat, lon:p.gpsLon})
            }
            if (calibrationWriter.save(selectedFile, cameraFrame.sourceSize.width,
                                       cameraFrame.sourceSize.height, points))
                panel.savedPath = selectedFile.toString()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            Label { text: "Calibration caméra → carte"; font.bold: true; font.pixelSize: 17; color: "#0f172a"; Layout.fillWidth: true }
            Button { text: "Fermer"; onClicked: { panel.visible = false; panel.pendingPixel = null } }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: "#334155"
            text: "Caméra fixe, sol approximativement plan. Cliquer un repère au sol sur l'image, puis le même repère sur la carte. Répéter au moins 4 fois, autour de la zone à suivre."
        }
        Button { text: "Ouvrir l'image de la caméra…"; onClicked: imageDialog.open() }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 245
            color: "#182131"
            Image {
                id: cameraFrame
                anchors.fill: parent
                source: panel.imageSource
                fillMode: Image.PreserveAspectFit
                asynchronous: false
                Repeater {
                    model: references
                    Rectangle {
                        x: (cameraFrame.width - cameraFrame.paintedWidth)/2 + pixelX/cameraFrame.sourceSize.width*cameraFrame.paintedWidth - 10
                        y: (cameraFrame.height - cameraFrame.paintedHeight)/2 + pixelY/cameraFrame.sourceSize.height*cameraFrame.paintedHeight - 10
                        width: 20; height: 20; radius: 10; color: "#00d4ff"; border.color: "white"
                        Text { anchors.centerIn: parent; text: index + 1; font.bold: true; color: "#0f172a" }
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: cameraFrame.status === Image.Ready
                    cursorShape: Qt.CrossCursor
                    onClicked: (mouse) => {
                        const px = (mouse.x - (cameraFrame.width-cameraFrame.paintedWidth)/2) * cameraFrame.sourceSize.width/cameraFrame.paintedWidth
                        const py = (mouse.y - (cameraFrame.height-cameraFrame.paintedHeight)/2) * cameraFrame.sourceSize.height/cameraFrame.paintedHeight
                        if (px >= 0 && py >= 0 && px < cameraFrame.sourceSize.width && py < cameraFrame.sourceSize.height)
                            panel.pendingPixel = Qt.point(px,py)
                    }
                }
            }
            Label { visible: cameraFrame.status !== Image.Ready; anchors.centerIn: parent; text: "Ouvrir une image extraite de la vidéo"; color: "#cbd5e1" }
        }
        Label {
            Layout.fillWidth: true; wrapMode: Text.WordWrap
            color: panel.waitingForMap ? "#b45309" : "#334155"
            text: panel.waitingForMap ? "→ Cliquer maintenant le même repère sur la carte." : references.count + " correspondances enregistrées."
        }
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 50
            clip: true; model: references
            delegate: Label {
                width: ListView.view.width; height: 25; color: "#334155"; font.pixelSize: 12
                text: (index+1) + " · pixel " + pixelX.toFixed(0) + ", " + pixelY.toFixed(0)
                    + " → " + gpsLat.toFixed(6) + ", " + gpsLon.toFixed(6)
            }
        }
        RowLayout {
            Button {
                text: "Annuler le dernier point"
                enabled: references.count > 0 || panel.waitingForMap
                onClicked: {
                    if (panel.waitingForMap) panel.pendingPixel = null
                    else references.remove(references.count - 1)
                    panel.savedPath = ""
                }
            }
            Button { text: "Enregistrer…"; enabled: references.count >= 4; onClicked: saveDialog.open() }
        }
        Label {
            Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize: 11
            color: panel.savedPath ? "#15803d" : "#9f1239"
            text: panel.savedPath ? "Repères enregistrés. Le module Python calculera et contrôlera l'homographie."
                 : calibrationWriter.error
        }
    }
}

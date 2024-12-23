import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import com.company.VerilogLintChecker

Window {
    width: 800
    height: 600
    visible: true
    title: qsTr("VerilogLinter")

    property string fileName
    property bool arithmetic
    property bool unReachableBlck
    property bool unReachableFSM
    property bool unInitReg
    property bool busReg
    property bool fullParCase
    property bool latches

    Connections {
        target: VerilogLintChecker
        function onFileReady() {
            scene1.visible = false
            fileModel.clear()
            VerilogLintChecker.getFileContent().forEach(
                        line => fileModel.append({
                                                     "line": line
                                                 }))
            scene2.visible = true
        }
    }

    Rectangle {
        id: scene1
        anchors.fill: parent
        color: "#4a4848"

        Image {
            id: logo
            source: "assets/logo.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
            width: 100
            height: 100
            anchors {
                top: parent.top
                right: parent.right
            }
        }
        Text {
            id: text1
            color: "white"
            anchors {
                left: parent.left
                top: parent.top
                margins: 70
            }
            font.pointSize: 18
            text: "Choose the verilog file to be checked:"
        }
        Rectangle {
            width: 600
            height: 50
            id: filechooser
            color: "grey"
            anchors {
                left: parent.left
                top: text1.bottom
                margins: 10
            }
            clip: true
            Text {
                id: text2
                color: "turquoise"
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }

                anchors.margins: 10
                font.pointSize: 15
                text: "no file chosen"
                clip: true
            }
        }

        Button {
            id: button1
            width: 400
            height: 20
            text: "Choose File"
            anchors {
                top: filechooser.bottom
                horizontalCenter: parent.horizontalCenter
                margins: 10
            }

            onClicked: fileDialog.open() // Trigger the file dialog
        }

        FileDialog {
            id: fileDialog
            title: "Select a File"
            folder: "C:\Users" // Set the default folder
            onAccepted: {
                console.log("File selected:", file)
                text2.text = file
                fileName = file
                //VerilogLintChecker.parseFile(file)
            }
            onRejected: {
                console.log("File selection canceled")
            }
        }

        Column {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: button1.bottom
                topMargin: 20
            }

            Text {
                text: "Choose Checks:"
                font {
                    bold: true
                    pointSize: 13
                }
                anchors {
                    left: parent.left
                }
                color: "white"
            }
            CheckBox {
                id: all
                text: "All"
                height: 25
                contentItem: Text {
                    text: all.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: all.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: all.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
                checkState: "Checked"
            }
            CheckBox {
                id: unreachable_Blocks
                text: "Unreachable Blocks "
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: unreachable_Blocks.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: unreachable_Blocks.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: unreachable_Blocks.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: unreachable_FSM_State
                text: "Unreachable FSM State "
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: unreachable_FSM_State.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: unreachable_FSM_State.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: unreachable_FSM_State.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: uninitialized_Register
                text: "Un-initialized Register"
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: uninitialized_Register.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: uninitialized_Register.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: uninitialized_Register.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: multiDriven_Bus_Register
                text: "Multi-Driven Bus/Register "
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: multiDriven_Bus_Register.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: multiDriven_Bus_Register.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: multiDriven_Bus_Register.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: full_Parallel_Case
                text: "Non Full/Parallel Case"
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: full_Parallel_Case.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: full_Parallel_Case.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: full_Parallel_Case.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: inferLatch
                text: "Inferred Latches"
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: inferLatch.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: inferLatch.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: inferLatch.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }
            CheckBox {
                id: arithmeticOverflow
                text: "Arithmetic Overflow"
                height: 25
                enabled: !all.checkState
                opacity: !all.checkState ? 1 : 0.2
                contentItem: Text {
                    text: arithmeticOverflow.text
                    color: "turquoise"
                    font.pixelSize: 18
                    anchors.left: arithmeticOverflow.left
                    anchors.leftMargin: 45
                    anchors.verticalCenter: arithmeticOverflow.verticalCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }

            Button {
                id: button2
                width: 400
                height: 20
                text: "Generate Report"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    arithmetic = arithmeticOverflow.checkState || all.checkState
                    unReachableBlck = unreachable_Blocks.checkState
                            || all.checkState
                    unReachableFSM = unreachable_FSM_State.checkState
                            || all.checkState
                    unInitReg = uninitialized_Register.checkState
                            || all.checkState
                    busReg = multiDriven_Bus_Register.checkState
                            || all.checkState
                    fullParCase = full_Parallel_Case.checkState
                            || all.checkState
                    latches = inferLatch.checkState || all.checkState
                    if (fileName.length != 0) {
                        VerilogLintChecker.parseFile(fileName, arithmetic,
                                                     unReachableBlck,
                                                     unReachableFSM,
                                                     unInitReg, busReg,
                                                     fullParCase, latches)
                    }
                }
            }
        }
    }

    Rectangle {
        id: scene2
        anchors.fill: parent
        color: "#4a4848"
        visible: false

        Button {
            id: button3
            width: 400
            height: 20
            text: "Back"
            anchors {
                top: parent.top
                left: parent.left
                margins: 10
            }

            onClicked: {
                scene1.visible = true
                scene2.visible = false
            }
        }

        Image {
            id: logo1
            source: "assets/logo.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
            width: 100
            height: 100
            anchors {
                top: parent.top
                right: parent.right
            }
        }
        ListView {
            id: listView
            width: parent.width
            height: parent.height - 100
            anchors.bottom: parent.bottom
            model: ListModel {
                id: fileModel
            }

            delegate: Rectangle {
                width: listView.width
                height: 30
                color: index % 4 === 0 ? "lightgray" : "grey"

                Text {
                    anchors.centerIn: parent
                    text: model.line
                    elide: Text.ElideRight
                }
            }

            clip: true
        }
    }
}

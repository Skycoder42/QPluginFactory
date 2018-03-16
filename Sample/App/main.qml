import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ApplicationWindow {
	visible: true
	width: 640
	height: 480
	title: qsTr("Plugin loader test")

	ListView {
		anchors.fill: parent
		model: plugin.keys

		delegate: ItemDelegate {
			width: parent.width
			text: modelData

			onPressAndHold: plugin.info(modelData)
			onClicked: plugin.load(modelData)
		}

		footer: GridLayout {
			width: parent.width
			columns: 2

			Label {
				text: "Title"
				Layout.fillWidth: true
			}

			TextField {
				text: plugin.title
				readOnly: true
				Layout.fillWidth: true
			}

			Label {
				text: "Ping"
				Layout.fillWidth: true
			}

			TextField {
				id: pingText
				placeholderText: "Enter a ping text"
				onTextChanged: plugin.ping(text)
				Layout.fillWidth: true

				Connections {
					target: plugin
					onUpdated: pingText.clear()
				}
			}

			Label {
				text: "Pong"
				Layout.fillWidth: true
			}

			TextField {
				id: pongText
				placeholderText: "wait for the pong"
				onTextChanged: plugin.ping(text)
				readOnly: true
				Layout.fillWidth: true

				Connections {
					target: plugin
					onPong: pongText.text = text
				}
			}
		}
	}
}

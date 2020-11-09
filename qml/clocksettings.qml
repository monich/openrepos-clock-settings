import QtQuick 2.0
import Sailfish.Silica 1.0
import openrepos.clock.settings 1.0

Page {
    id: page

    //: Clock settings page header
    //% "Clock"
    property string applicationName: qsTrId("settings_clock-he-clock")

    ClockSettings {
        id: settings

        alarmPlaying: volumeSlider.down
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: parent.width

            PageHeader {
                title: page.applicationName
            }

            Slider {
                width: parent.width
                visible: opacity > 0
                opacity: settings.snoozeQueryFinished ? 1 : 0
                Behavior on opacity { FadeAnimation {} }
                minimumValue: ClockSettings.SnoozeMin
                maximumValue: ClockSettings.SnoozeMax
                stepSize: ClockSettings.SnoozeStep
                //: Minutes (shortened)
                //% "min"
                valueText: value/60 + " " + qsTrId("settings_clock-la-min")
                //: Slider label
                //% "Snooze interval"
                label: qsTrId("settings_clock-la-snooze-interval")
                value: settings.snooze
                onSliderValueChanged: settings.snooze = value
            }

            Slider {
                id: volumeSlider

                width: parent.width
                visible: opacity > 0
                opacity: (settings.snoozeQueryFinished && settings.volumeQueryFinished) ? 1 : 0
                Behavior on opacity { FadeAnimation {} }
                minimumValue: ClockSettings.VolumeMin
                maximumValue: ClockSettings.VolumeMax
                stepSize: ClockSettings.VolumeStep
                valueText: value
                //: Slider label
                //% "Alarm volume"
                label: qsTrId("settings_clock-la-alarm-volume")
                value: settings.volume
                onSliderValueChanged: settings.volume = value
            }
        }
    }
}

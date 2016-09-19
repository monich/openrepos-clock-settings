/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import openrepos.clock.settings 1.0

Page {
    id: page

    ClockSettings {
        id: settings
        alarmPlaying: volumeSlider.down
        onSnoozeChanged: snoozeSlider.value = snooze
        onVolumeChanged: volumeSlider.value = volume
    }

    Component.onCompleted: {
        snoozeSlider.value = settings.snooze
        volumeSlider.value = settings.volume
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            PageHeader {
                //: Clock settings page header
                //% "Clock"
                title: qsTrId("settings_clock-he-clock")
            }

            Slider {
                id: snoozeSlider
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
                width: page.width
                onSliderValueChanged: settings.snooze = value
            }

            Slider {
                id: volumeSlider
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
                width: page.width
                onSliderValueChanged: settings.volume = value
            }
        }
    }
}

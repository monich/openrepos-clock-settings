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

#ifndef CLOCKSETTINGS_H
#define CLOCKSETTINGS_H

#include "profilevalueinfo.h"

class TimeDaemon;
class ProfileDaemon;
class FeedbackDaemon;
class QDBusPendingCallWatcher;

class ClockSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(SnoozeConstants)
    Q_ENUMS(VolumeConstants)
    Q_PROPERTY(int snooze READ snooze WRITE setSnooze NOTIFY snoozeChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool alarmPlaying READ alarmPlaying WRITE setAlarmPlaying NOTIFY alarmPlayingChanged)
    Q_PROPERTY(bool snoozeQueryFinished READ snoozeQueryFinished NOTIFY snoozeQueryFinishedChanged)
    Q_PROPERTY(bool volumeQueryFinished READ volumeQueryFinished NOTIFY volumeQueryFinishedChanged)

public:
    enum SnoozeConstants {
        SnoozeMin = 60,
        SnoozeMax = 1800,
        SnoozeStep = 60,
        SnoozeDefault = 300
    };

    enum VolumeConstants {
        VolumeMin = 0,
        VolumeMax = 100,
        VolumeStep = 1,
        VolumeDefault = 100
    };

    ClockSettings(QObject* aParent = NULL);

    int snooze() const;
    void setSnooze(int aValue);
    bool snoozeQueryFinished() const;

    int volume() const;
    void setVolume(int aValue);
    bool volumeQueryFinished() const;

    bool alarmPlaying() const;
    void setAlarmPlaying(bool aValue);

private:
    void queryVolume();
    void stopPlaying();

private Q_SLOTS:
    void onGetProfilesFinished(QDBusPendingCallWatcher* aCall);
    void onGetProfileFinished(QDBusPendingCallWatcher* aCall);
    void onGetVolumeFinished(QDBusPendingCallWatcher* aCall);
    void onGetAppSnoozeFinished(QDBusPendingCallWatcher* aCall);
    void onPlayFinished(QDBusPendingCallWatcher* aCall);
    void onProfileChanged(bool aChanged, bool aActive, QString aProfile,
        ProfileValueInfoList aValues);

Q_SIGNALS:
    void snoozeChanged();
    void volumeChanged();
    void alarmPlayingChanged();
    void snoozeQueryFinishedChanged();
    void volumeQueryFinishedChanged();

private:
    int iSnooze;
    int iVolume;
    bool iAlarmPlaying;
    QString iAlarmEvent;
    uint iAlarmEventId;
    bool iSnoozeQueryFinished;
    bool iVolumeQueryFinished;
    QString iClockApp;
    QString iVolumeKey;
    QString iProfile;
    QStringList iProfileList;
    TimeDaemon* iTimeDaemon;
    ProfileDaemon* iProfileDaemon;
    FeedbackDaemon* iFeedbackDaemon;
};

inline int ClockSettings::snooze() const
    { return iSnooze; }
inline int ClockSettings::volume() const
    { return iVolume; }
inline bool ClockSettings::alarmPlaying() const
    { return iAlarmPlaying; }
inline bool ClockSettings::snoozeQueryFinished() const
    { return iSnoozeQueryFinished; }
inline bool ClockSettings::volumeQueryFinished() const
    { return iVolumeQueryFinished; }

#endif // EPIPHANGRABBER_H

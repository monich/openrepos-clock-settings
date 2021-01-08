/*
 * Copyright (C) 2021 Jolla Ltd.
 * Copyright (C) 2021 Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

#ifndef VOLUMESETTINGS_H
#define VOLUMESETTINGS_H

#include "profilevalueinfo.h"

#include <QStringList>

class ProfileDaemon;
class FeedbackDaemon;
class QDBusPendingCallWatcher;

class VolumeSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(VolumeConstants)
    Q_PROPERTY(QString alarmEvent READ alarmEvent WRITE setAlarmEvent NOTIFY alarmEventChanged)
    Q_PROPERTY(QString volumeKey READ volumeKey WRITE setVolumeKey NOTIFY volumeKeyChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool alarmPlaying READ alarmPlaying WRITE setAlarmPlaying NOTIFY alarmPlayingChanged)
    Q_PROPERTY(bool volumeQueryFinished READ volumeQueryFinished NOTIFY volumeQueryFinishedChanged)

public:
    enum VolumeConstants {
        VolumeMin = 0,
        VolumeMax = 100,
        VolumeStep = 1,
        VolumeDefault = 100
    };

    VolumeSettings(QObject* aParent = Q_NULLPTR);

    QString alarmEvent() const;
    void setAlarmEvent(QString aEvent);

    QString volumeKey() const;
    void setVolumeKey(QString aKey);

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
    void onPlayFinished(QDBusPendingCallWatcher* aCall);
    void onProfileChanged(bool aChanged, bool aActive, QString aProfile,
        ProfileValueInfoList aValues);

Q_SIGNALS:
    void alarmEventChanged();
    void volumeKeyChanged();
    void volumeChanged();
    void alarmPlayingChanged();
    void volumeQueryFinishedChanged();

private:
    int iVolume;
    bool iAlarmPlaying;
    QString iAlarmEvent;
    uint iAlarmEventId;
    bool iVolumeQueryFinished;
    QString iVolumeKey;
    QString iProfile;
    QStringList iProfileList;
    ProfileDaemon* iProfileDaemon;
    FeedbackDaemon* iFeedbackDaemon;
    QDBusPendingCallWatcher* iProfileQuery;
    QDBusPendingCallWatcher* iVolumeQuery;
};

inline QString VolumeSettings::alarmEvent() const
    { return iAlarmEvent; }
inline QString VolumeSettings::volumeKey() const
    { return iVolumeKey; }
inline int VolumeSettings::volume() const
    { return iVolume; }
inline bool VolumeSettings::alarmPlaying() const
    { return iAlarmPlaying; }
inline bool VolumeSettings::volumeQueryFinished() const
    { return iVolumeQueryFinished; }

#endif // CLOCKSETTINGS_H

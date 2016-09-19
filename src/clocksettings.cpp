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

#include "clocksettings.h"
#include "time_interface.h"
#include "profiled_interface.h"
#include "nongraphicfeedback1_interface.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

#define WARN(x) qWarning() << x
#ifdef DEBUGLOG
#  define DBG(x) qDebug() << x
#else
#  define DBG(expr) ((void)0)
#endif

#define TIMED_CONNECTION       QDBusConnection::systemBus()
#define TIMED_SERVICE          "com.nokia.time"
#define TIMED_PATH             "/com/nokia/time"
#define CLOCK_APP             "nemoalarms"

#define PROFILED_CONNECTION    QDBusConnection::sessionBus()
#define PROFILED_SERVICE       "com.nokia.profiled"
#define PROFILED_PATH          "/com/nokia/profiled"
#define DEFAULT_PROFILE        "general"
#define ALARM_VOLUME_KEY       "clock.alert.volume"

#define NGFD_CONNECTION         QDBusConnection::systemBus()
#define NGFD_SERVICE            "com.nokia.NonGraphicFeedback1.Backend"
#define NGFD_PATH               "/com/nokia/NonGraphicFeedback1"
#define NGFD_ALARM_EVENT        "clock"

ClockSettings::ClockSettings(QObject* aParent) :
    QObject(aParent),
    iSnooze(SnoozeDefault),
    iVolume(VolumeDefault),
    iAlarmPlaying(false),
    iAlarmEvent(NGFD_ALARM_EVENT),
    iAlarmEventId(0),
    iSnoozeQueryFinished(false),
    iVolumeQueryFinished(false),
    iClockApp(CLOCK_APP),
    iVolumeKey(ALARM_VOLUME_KEY),
    iTimeDaemon(new TimeDaemon(TIMED_SERVICE, TIMED_PATH, TIMED_CONNECTION, this)),
    iProfileDaemon(new ProfileDaemon(PROFILED_SERVICE, PROFILED_PATH, PROFILED_CONNECTION, this)),
    iFeedbackDaemon(new FeedbackDaemon(NGFD_SERVICE, NGFD_PATH, NGFD_CONNECTION, this))
{
    connect(new QDBusPendingCallWatcher(
        iTimeDaemon->get_app_snooze(iClockApp), iTimeDaemon),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetAppSnoozeFinished(QDBusPendingCallWatcher*)));
    connect(new QDBusPendingCallWatcher(
        iProfileDaemon->get_profiles(), iProfileDaemon),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetProfilesFinished(QDBusPendingCallWatcher*)));
    connect(iProfileDaemon,
        SIGNAL(profile_changed(bool,bool,QString,ProfileValueInfoList)),
        SLOT(onProfileChanged(bool,bool,QString,ProfileValueInfoList)));
}

void ClockSettings::queryVolume()
{
    DBG("querying alarm volume for" << iProfile);
    connect(new QDBusPendingCallWatcher(
        iProfileDaemon->get_value(iProfile, iVolumeKey), iProfileDaemon),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetVolumeFinished(QDBusPendingCallWatcher*)));
}

void ClockSettings::onGetProfilesFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QStringList> reply = *aCall;
    if (reply.isError()) {
        WARN(reply.error());
    } else {
        iProfileList = reply.value();
        DBG(iProfileList);
        if (iProfileList.contains(DEFAULT_PROFILE)) {
            iProfile = DEFAULT_PROFILE;
            queryVolume();
        } else {
            connect(new QDBusPendingCallWatcher(
                iProfileDaemon->get_profile(), iProfileDaemon),
                SIGNAL(finished(QDBusPendingCallWatcher*)),
                SLOT(onGetProfileFinished(QDBusPendingCallWatcher*)));
        }
    }
    aCall->deleteLater();
}

void ClockSettings::onGetProfileFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QString> reply = *aCall;
    if (reply.isError()) {
        WARN(reply.error());
        iProfile = DEFAULT_PROFILE;
    } else {
        iProfile = reply.value();
    }
    queryVolume();
    aCall->deleteLater();
}

void ClockSettings::onGetVolumeFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QString> reply = *aCall;
    if (reply.isError()) {
        WARN(reply.error());
    } else {
        QString str(reply.value());
        DBG(str);
        bool ok = false;
        const int volume = str.toInt(&ok);
        if (ok) {
            if (iVolume != volume) {
                iVolume = volume;
                Q_EMIT volumeChanged();
            }
            iVolumeQueryFinished = true;
            Q_EMIT volumeQueryFinishedChanged();
        }
    }
    aCall->deleteLater();
}

void ClockSettings::onGetAppSnoozeFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<int> reply = *aCall;
    if (reply.isError()) {
        WARN(reply.error());
    } else {
        const int value = reply.value();
        DBG(value);
        iSnoozeQueryFinished = true;
        if (iSnooze != value) {
            iSnooze = value;
            Q_EMIT snoozeChanged();
        }
        Q_EMIT snoozeQueryFinishedChanged();
    }
    aCall->deleteLater();
}

void ClockSettings::onProfileChanged(bool aChanged, bool aActive, QString aProfile,
    ProfileValueInfoList aValues)
{
    DBG(aProfile << aValues);
    if (aProfile == iProfile) {
        const int n = aValues.count();
        for (int i=0; i<n; i++) {
            const ProfileValueInfo& value = aValues.at(i);
            if (value.key == iVolumeKey) {
                bool ok = false;
                const int volume = value.value.toInt(&ok);
                if (ok) {
                    if (iVolume != volume) {
                        iVolume = volume;
                        Q_EMIT volumeChanged();
                    }
                }
                break;
            }
        }
    }
}

void ClockSettings::setSnooze(int aValue)
{
    if (aValue >= SnoozeMin &&
        aValue <= SnoozeMax &&
        iSnooze != aValue) {
        iSnooze = aValue;
        DBG(iSnooze);
        iTimeDaemon->set_app_snooze(iClockApp, iSnooze);
        Q_EMIT snoozeChanged();
    }
}

void ClockSettings::setVolume(int aValue)
{
    if (aValue >= VolumeMin &&
        aValue <= VolumeMax &&
        iVolume != aValue) {
        iVolume = aValue;
        DBG(iVolume);

        // Alarm volume seems to be using the "general" volume regardless of
        // the currently active profile. So let's update alarm volume for all
        // profiles. Just in case.
        QString value(QString::number(iVolume));
        const int n = iProfileList.count();
        for (int i=0; i<n; i++) {
            iProfileDaemon->set_value(iProfileList.at(i), iVolumeKey, value);
        }
        Q_EMIT volumeChanged();
    }
}

void ClockSettings::setAlarmPlaying(bool aValue)
{
    if (aValue) {
        if (!iAlarmPlaying) {
            iAlarmPlaying = true;
            stopPlaying();
            Q_EMIT alarmPlayingChanged();
            connect(new QDBusPendingCallWatcher(
                iFeedbackDaemon->Play(iAlarmEvent, QVariantMap()), iFeedbackDaemon),
                SIGNAL(finished(QDBusPendingCallWatcher*)),
                SLOT(onPlayFinished(QDBusPendingCallWatcher*)));
        }
    } else {
        stopPlaying();
        if (iAlarmPlaying) {
            iAlarmPlaying = false;
            Q_EMIT alarmPlayingChanged();
        }
    }
}

void ClockSettings::stopPlaying()
{
    if (iAlarmEventId) {
        DBG("stopping feedback");
        iFeedbackDaemon->Stop(iAlarmEventId);
        iAlarmEventId = 0;
    }
}

void ClockSettings::onPlayFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<uint> reply = *aCall;
    if (reply.isError()) {
        WARN(reply.error());
    } else {
        const uint eventId =  reply.value();
        if (iAlarmPlaying) {
            DBG("alarm id" << eventId);
            iAlarmEventId = eventId;
        } else {
            DBG("alarm id" << eventId << "(never mind)");
            iFeedbackDaemon->Stop(eventId);
        }
    }
    aCall->deleteLater();
}

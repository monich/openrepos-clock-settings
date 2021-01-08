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

#include "volumesettings.h"
#include "profiled_interface.h"
#include "nongraphicfeedback1_interface.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

#define WARN(x) qWarning() << x
#ifdef DEBUGLOG
#  define DBG(expr) qDebug() << expr
#  define ASSERT(expr) Q_ASSERT(expr)
#else
#  define DBG(expr) ((void)0)
#  define ASSERT(expr) ((void)0)
#endif

namespace {
    const QString DEFAULT_PROFILE("general");

#define PROFILED_CONNECTION QDBusConnection::sessionBus()
    const QString PROFILED_SERVICE("com.nokia.profiled");
    const QString PROFILED_PATH("/com/nokia/profiled");

#define NGFD_CONNECTION QDBusConnection::systemBus()
    const QString NGFD_SERVICE("com.nokia.NonGraphicFeedback1.Backend");
    const QString NGFD_PATH("/com/nokia/NonGraphicFeedback1");
}

VolumeSettings::VolumeSettings(QObject* aParent) :
    QObject(aParent),
    iVolume(VolumeDefault),
    iAlarmPlaying(false),
    iAlarmEventId(0),
    iVolumeQueryFinished(false),
    iProfileDaemon(new ProfileDaemon(PROFILED_SERVICE, PROFILED_PATH, PROFILED_CONNECTION, this)),
    iFeedbackDaemon(new FeedbackDaemon(NGFD_SERVICE, NGFD_PATH, NGFD_CONNECTION, this)),
    iProfileQuery(Q_NULLPTR),
    iVolumeQuery(Q_NULLPTR)
{
    connect(iProfileDaemon,
        SIGNAL(profile_changed(bool,bool,QString,ProfileValueInfoList)),
        SLOT(onProfileChanged(bool,bool,QString,ProfileValueInfoList)));
    connect(iProfileQuery = new QDBusPendingCallWatcher(
        iProfileDaemon->get_profiles(), iProfileDaemon),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetProfilesFinished(QDBusPendingCallWatcher*)));
}

void VolumeSettings::onGetProfilesFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QStringList> reply = *aCall;

    ASSERT(iProfileQuery == aCall);
    iProfileQuery = Q_NULLPTR;
    aCall->deleteLater();

    if (reply.isError()) {
        WARN(reply.error());
    } else {
        iProfileList = reply.value();
        DBG(iProfileList);
        if (iProfileList.contains(DEFAULT_PROFILE)) {
            iProfile = DEFAULT_PROFILE;
            queryVolume();
        } else {
            connect(iProfileQuery = new QDBusPendingCallWatcher(
                iProfileDaemon->get_profile(), iProfileDaemon),
                SIGNAL(finished(QDBusPendingCallWatcher*)),
                SLOT(onGetProfileFinished(QDBusPendingCallWatcher*)));
        }
    }
}

void VolumeSettings::onGetProfileFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QString> reply = *aCall;

    ASSERT(iProfileQuery == aCall);
    iProfileQuery = Q_NULLPTR;
    aCall->deleteLater();

    if (reply.isError()) {
        WARN(reply.error());
        iProfile = DEFAULT_PROFILE;
    } else {
        iProfile = reply.value();
    }
    DBG(iProfile);
    queryVolume();
}

void VolumeSettings::queryVolume()
{
    if (!iProfileQuery && !iProfile.isEmpty() && !iVolumeKey.isEmpty()) {
        DBG("querying" << qPrintable(iVolumeKey) << "for" << qPrintable(iProfile));
        delete iVolumeQuery;
        iVolumeQuery = Q_NULLPTR;
        connect(iVolumeQuery = new QDBusPendingCallWatcher(
            iProfileDaemon->get_value(iProfile, iVolumeKey), iProfileDaemon),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onGetVolumeFinished(QDBusPendingCallWatcher*)));
    }
}

void VolumeSettings::onGetVolumeFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<QString> reply = *aCall;

    ASSERT(iVolumeQuery == aCall);
    iVolumeQuery = Q_NULLPTR;
    aCall->deleteLater();

    if (reply.isError()) {
        WARN(reply.error());
    } else {
        const QString str(reply.value());
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
}

void VolumeSettings::onProfileChanged(bool aChanged, bool aActive,
    QString aProfile, ProfileValueInfoList aValues)
{
    DBG(aProfile << aValues);
    if (aProfile == iProfile) {
        const int n = aValues.count();
        for (int i = 0; i < n; i++) {
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

void VolumeSettings::setAlarmEvent(QString aEvent)
{
    if (iAlarmEvent != aEvent) {
        iAlarmEvent = aEvent;
        DBG(iAlarmEvent);
        Q_EMIT alarmEventChanged();
    }
}

void VolumeSettings::setVolumeKey(QString aKey)
{
    if (iVolumeKey != aKey) {
        iVolumeKey = aKey;
        DBG(iVolumeKey);
        queryVolume();
        if (iVolumeQueryFinished) {
            iVolumeQueryFinished = false;
            Q_EMIT volumeQueryFinishedChanged();
        }
        Q_EMIT volumeKeyChanged();
    }
}

void VolumeSettings::setVolume(int aValue)
{
    const int value = (aValue < VolumeMin) ? VolumeMin :
        (aValue > VolumeMax) ? VolumeMax : aValue;
    if (iVolume != value) {
        iVolume = value;
        DBG(iVolume);

        // Alarm volume seems to be using the "general" volume regardless of
        // the currently active profile. So let's update alarm volume for all
        // profiles. Just in case.
        QString value(QString::number(iVolume));
        const int n = iProfileList.count();
        for (int i = 0; i < n; i++) {
            iProfileDaemon->set_value(iProfileList.at(i), iVolumeKey, value);
        }
        Q_EMIT volumeChanged();
    }
}

void VolumeSettings::setAlarmPlaying(bool aValue)
{
    if (aValue) {
        if (!iAlarmEvent.isEmpty() && !iAlarmPlaying) {
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

void VolumeSettings::stopPlaying()
{
    if (iAlarmEventId) {
        DBG("stopping feedback");
        iFeedbackDaemon->Stop(iAlarmEventId);
        iAlarmEventId = 0;
    }
}

void VolumeSettings::onPlayFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<uint> reply = *aCall;
    aCall->deleteLater();
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
}

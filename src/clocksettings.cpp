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

#include <QDebug>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

#define DBUS_CONNECTION QDBusConnection::systemBus()
#define DBUS_PATH       "/com/nokia/time"
#define DBUS_SERVICE    "com.nokia.time"
#define APPLICATION     "nemoalarms"

ClockSettings::ClockSettings(QObject* aParent) :
    QObject(aParent),
    iSnooze(DefaultSnoozeInterval),
    iSnoozeChanged(false),
    iQueryPending(true),
    iClockApp(APPLICATION),
    iTimeDaemon(new TimeDaemon(DBUS_SERVICE, DBUS_PATH, DBUS_CONNECTION, this))
{
    connect(new QDBusPendingCallWatcher(
        iTimeDaemon->get_app_snooze(iClockApp), iTimeDaemon),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onQueryFinished(QDBusPendingCallWatcher*)));
}

void ClockSettings::onQueryFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<int> reply = *aCall;
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        const int value = reply.value();
        qDebug() << value;
        iQueryPending = false;
        if (!iSnoozeChanged && iSnooze != value) {
            iSnooze = value;
            Q_EMIT snoozeChanged();
        }
        Q_EMIT queryPendingChanged();
    }
    aCall->deleteLater();
}

void ClockSettings::setSnooze(int aValue)
{
    if (aValue >= MinSnoozeInterval &&
        aValue <= MaxSnoozeInterval &&
        iSnooze != aValue) {
        iSnooze = aValue;
        iSnoozeChanged = true;
        qDebug() << iSnooze;
        iTimeDaemon->set_app_snooze(iClockApp, iSnooze);
        Q_EMIT snoozeChanged();
    }
}

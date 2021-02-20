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

#include "snoozesettings.h"
#include "time_interface.h"

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

#define TIMED_CONNECTION       QDBusConnection::systemBus()
#define TIMED_SERVICE          QStringLiteral("com.nokia.time")
#define TIMED_PATH             QStringLiteral("/com/nokia/time")

SnoozeSettings::SnoozeSettings(QObject* aParent) :
    QObject(aParent),
    iTimeDaemon(new TimeDaemon(TIMED_SERVICE, TIMED_PATH, TIMED_CONNECTION, this)),
    iGetAppSnoozeCall(Q_NULLPTR),
    iSnooze(SnoozeMin),
    iSnoozeQueryFinished(false)
{
}

void SnoozeSettings::setApplication(QString aApp)
{
    if (iApplication != aApp) {
        iApplication = aApp;
        if (iGetAppSnoozeCall) {
            delete iGetAppSnoozeCall;
            iGetAppSnoozeCall = Q_NULLPTR;
        }
        Q_EMIT applicationChanged();
        query();
    }
}

void SnoozeSettings::query()
{
    if (!iApplication.isEmpty()) {
        if (iGetAppSnoozeCall) {
            delete iGetAppSnoozeCall;
            iGetAppSnoozeCall = Q_NULLPTR;
        }
        connect(iGetAppSnoozeCall = new QDBusPendingCallWatcher(
            iTimeDaemon->get_app_snooze(iApplication), iTimeDaemon),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onGetAppSnoozeFinished(QDBusPendingCallWatcher*)));
        if (iSnoozeQueryFinished) {
            iSnoozeQueryFinished = false; // Not anymore
            Q_EMIT snoozeQueryFinishedChanged();
        }
    }
}

void SnoozeSettings::onGetAppSnoozeFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<int> reply = *aCall;

    ASSERT(iGetAppSnoozeCall == aCall);
    iGetAppSnoozeCall = NULL;
    aCall->deleteLater();

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
}

void SnoozeSettings::setSnooze(int aValue)
{
    const int value = (aValue < SnoozeMin) ? SnoozeMin :
        (aValue > SnoozeMax) ? SnoozeMax : aValue;
    if (iSnooze != value) {
        iSnooze = value;
        DBG(iSnooze);
        iTimeDaemon->set_app_snooze(iApplication, iSnooze);
        Q_EMIT snoozeChanged();
    }
}

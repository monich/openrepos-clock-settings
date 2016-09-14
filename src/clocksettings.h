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

#include <QObject>
#include <QString>

class TimeDaemon;
class QDBusPendingCallWatcher;

class ClockSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(Constants)
    Q_PROPERTY(bool queryPending READ queryPending NOTIFY queryPendingChanged)
    Q_PROPERTY(int snooze READ snooze WRITE setSnooze NOTIFY snoozeChanged)

public:
    enum Constants {
        MinSnoozeInterval = 60,
        MaxSnoozeInterval = 1800,
        DefaultSnoozeInterval = 300
    };

    ClockSettings(QObject* aParent = NULL);

    bool queryPending() const;
    int snooze() const;
    void setSnooze(int aValue);

private Q_SLOTS:
    void onQueryFinished(QDBusPendingCallWatcher* aCall);

Q_SIGNALS:
    void snoozeChanged();
    void queryPendingChanged();

private:
    int iSnooze;
    bool iSnoozeChanged;
    bool iQueryPending;
    QString iClockApp;
    TimeDaemon* iTimeDaemon;
};

inline bool ClockSettings::queryPending() const
    { return iQueryPending; }
inline int ClockSettings::snooze() const
    { return iSnooze; }

#endif // EPIPHANGRABBER_H

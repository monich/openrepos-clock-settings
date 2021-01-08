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

#ifndef SNOOZESETTINGS_H
#define SNOOZESETTINGS_H

#include <QString>
#include <QObject>

class TimeDaemon;
class QDBusPendingCallWatcher;

class SnoozeSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(SnoozeConstants)
    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY applicationChanged)
    Q_PROPERTY(int snooze READ snooze WRITE setSnooze NOTIFY snoozeChanged)
    Q_PROPERTY(bool snoozeQueryFinished READ snoozeQueryFinished NOTIFY snoozeQueryFinishedChanged)

public:
    enum SnoozeConstants {
        SnoozeMin = 60,
        SnoozeMax = 1800,
        SnoozeStep = 60,
        SnoozeDefault = 300
    };

    SnoozeSettings(QObject* aParent = Q_NULLPTR);

    QString application() const;
    void setApplication(QString aApp);

    int snooze() const;
    void setSnooze(int aValue);
    bool snoozeQueryFinished() const;

    Q_INVOKABLE void query();

private Q_SLOTS:
    void onGetAppSnoozeFinished(QDBusPendingCallWatcher* aCall);

Q_SIGNALS:
    void applicationChanged();
    void snoozeChanged();
    void snoozeQueryFinishedChanged();

private:
    TimeDaemon* iTimeDaemon;
    QDBusPendingCallWatcher* iGetAppSnoozeCall;
    QString iApplication;
    int iSnooze;
    bool iSnoozeQueryFinished;
};

inline QString SnoozeSettings::application() const
    { return iApplication; }
inline int SnoozeSettings::snooze() const
    { return iSnooze; }
inline bool SnoozeSettings::snoozeQueryFinished() const
    { return iSnoozeQueryFinished; }

#endif // SNOOZESETTINGS_H

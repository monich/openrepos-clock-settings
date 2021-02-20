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

#include "uninstaller.h"

#include <QDebug>
#include <QDBusPendingCallWatcher>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>

#define WARN(x) qWarning() << x
#ifdef DEBUGLOG
#  define DBG(expr) qDebug() << expr
#  define ASSERT(expr) Q_ASSERT(expr)
#else
#  define DBG(expr) ((void)0)
#  define ASSERT(expr) ((void)0)
#endif

#define DBUS_CONNECTION QDBusConnection::sessionBus()
static const QString DBUS_SERVICE("org.sailfishos.installationhandler");

class Uninstaller::Private : public QObject
{
    Q_OBJECT
    class Peer;
    class InstallationHandler;

public:
    Private(Uninstaller* aParent);
    ~Private();

    Uninstaller* parentObject();
    bool uninstall(QString aPackage);

private Q_SLOTS:
    void onPingFinished(QDBusPendingCallWatcher* aWatcher);
    void onRemovePackagesFinished(QDBusPendingCallWatcher* aWatcher);
#ifdef DEBUGLOG
    void onRemovalFinished(bool aSuccess, QString aError);
#endif

public:
    bool iAvailable;
    InstallationHandler* iInstallationHandler;
    QDBusPendingCallWatcher* iPendingCall;
    QString iPackage;
};

// ==========================================================================
// Uninstaller::Private::Peer
// ==========================================================================

class Uninstaller::Private::Peer: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    Peer(QObject* aParent) :
        QDBusAbstractInterface(DBUS_SERVICE, QStringLiteral("/"),
            "org.freedesktop.DBus.Peer", DBUS_CONNECTION, aParent) {}

public: // METHODS
    inline QDBusPendingCall Ping()
        { return asyncCall(QStringLiteral("Ping")); }
};

// ==========================================================================
// Uninstaller::Private::InstallationHandler
// ==========================================================================

class Uninstaller::Private::InstallationHandler: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    InstallationHandler(QObject* aParent) :
        QDBusAbstractInterface(DBUS_SERVICE,
            QStringLiteral("/org/sailfishos/installationhandler"),
            "org.sailfishos.installationhandler", DBUS_CONNECTION, aParent) {}

public: // METHODS
    inline QDBusPendingCall removePackages(QStringList aPackages)
        { return asyncCall(QStringLiteral("removePackages"), aPackages); }

Q_SIGNALS: // SIGNALS
    void removalFinished(bool success, QString errorString);
};

// ==========================================================================
// Uninstaller::Private
// ==========================================================================

Uninstaller::Private::Private(Uninstaller* aParent) :
    QObject(aParent),
    iAvailable(false),
    iInstallationHandler(Q_NULLPTR)
{
    // Ping the service to see if it's there
    Peer* peer = new Peer(this);
    iPendingCall = new QDBusPendingCallWatcher(peer->Ping(), peer);
    connect(iPendingCall, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onPingFinished(QDBusPendingCallWatcher*)));
}

Uninstaller::Private::~Private()
{
    delete iPendingCall;
    delete iInstallationHandler;
}

inline Uninstaller* Uninstaller::Private::parentObject()
{
    return qobject_cast<Uninstaller*>(parent());
}

void Uninstaller::Private::onPingFinished(QDBusPendingCallWatcher* aWatcher)
{
    Uninstaller* owner = parentObject();
    QDBusPendingReply<void> reply(*aWatcher);
    iPendingCall = Q_NULLPTR;
    if (reply.isError()) {
        WARN(reply.error());
    } else {
        DBG("Found" << qPrintable(DBUS_SERVICE));
        iAvailable = true;
        Q_EMIT owner->availableChanged();
    }
    Q_EMIT owner->busyChanged();
    // Delete the Peer which also deletes the watcher
    aWatcher->parent()->deleteLater();
}

void Uninstaller::Private::onRemovePackagesFinished(QDBusPendingCallWatcher* aWatcher)
{
    Uninstaller* owner = parentObject();
    QDBusPendingReply<void> reply(*aWatcher);
    iPendingCall = Q_NULLPTR;
    if (reply.isError()) {
        WARN(reply.error());
        Q_EMIT owner->uninstallFinished(false, reply.error().message());
    }
    Q_EMIT owner->busyChanged();
    aWatcher->deleteLater();
}

#ifdef DEBUGLOG
void Uninstaller::Private::onRemovalFinished(bool aSuccess, QString aError)
{
    DBG("Uninstallation" << (aSuccess ? "OK" : "ERROR"));
}
#endif

bool Uninstaller::Private::uninstall(QString aPackage)
{
    if (iAvailable) {
        Uninstaller* owner = parentObject();
        iPackage = aPackage;
        if (!iInstallationHandler) {
            iInstallationHandler = new InstallationHandler(this);
#ifdef DEBUGLOG
            connect(iInstallationHandler, SIGNAL(removalFinished(bool,QString)),
                SLOT(onRemovalFinished(bool,QString)));
#endif
            connect(iInstallationHandler, SIGNAL(removalFinished(bool,QString)),
                owner, SIGNAL(uninstallFinished(bool,QString)));
        }
        delete iPendingCall;
        DBG("Uninstalling" << aPackage);
        iPendingCall = new QDBusPendingCallWatcher(iInstallationHandler->removePackages(QStringList(aPackage)), this);
        connect(iPendingCall, SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onRemovePackagesFinished(QDBusPendingCallWatcher*)));
        Q_EMIT owner->busyChanged();
        return true;
    }
    return false;
}

// ==========================================================================
// Uninstaller
// ==========================================================================

Uninstaller::Uninstaller(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

Uninstaller::~Uninstaller()
{
    delete iPrivate;
}

bool Uninstaller::available() const
{
    return iPrivate->iAvailable;
}

bool Uninstaller::busy() const
{
    return iPrivate->iPendingCall != Q_NULLPTR;
}

bool Uninstaller::uninstall(QString aPackage)
{
    return iPrivate->uninstall(aPackage);
}

#include "uninstaller.moc"

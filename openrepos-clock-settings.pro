TEMPLATE = lib
TARGET = clocksettings

PREFIX = openrepos
NAME = clock-settings
TARGETPATH = $$[QT_INSTALL_QML]/$${PREFIX}/clock/settings

QT += qml dbus
CONFIG += plugin
QMAKE_CXXFLAGS += -Wno-unused-parameter
INCLUDEPATH += src

CONFIG(debug, debug|release) {
    DEFINES += DEBUGLOG
}

import.files = src/qmldir
import.path = $$TARGETPATH
target.path = $$TARGETPATH

settings_qml.path = /usr/share/$${PREFIX}-$${NAME}
settings_qml.files = qml/*.qml

settings_json.path = /usr/share/jolla-settings/entries
settings_json.files = $${PREFIX}-$${NAME}.json

OTHER_FILES += \
  *.json \
  translations/*.ts \
  src/*.xml \
  qml/*.qml \
  rpm/*.spec

SOURCES += \
  src/clocksettings.cpp \
  src/clocksettingsplugin.cpp \
  src/profilevalueinfo.cpp

HEADERS += \
  src/clocksettings.h \
  src/clocksettingsplugin.h \
  src/profilevalueinfo.h

INSTALLS += target import settings_qml settings_json

# D-Bus interfaces
timed.files = src/com.nokia.time.xml
timed.header_flags = -N -c TimeDaemon
timed.source_flags = -N -c TimeDaemon
DBUS_INTERFACES += timed

profiled.files = src/com.nokia.profiled.xml
profiled.header_flags = -N -c ProfileDaemon -i profilevalueinfo.h
profiled.source_flags = -N -c ProfileDaemon
DBUS_INTERFACES += profiled

ngfd.files = src/com.nokia.NonGraphicFeedback1.xml
ngfd.header_flags = -N -c FeedbackDaemon
ngfd.source_flags = -N -c FeedbackDaemon
DBUS_INTERFACES += ngfd

# Translations
TRANSLATIONS_PATH = /usr/share/translations
TRANSLATION_SOURCES = $${_PRO_FILE_PWD_}/qml

defineTest(addTrFile) {
    in = $${_PRO_FILE_PWD_}/translations/$${PREFIX}-$$1
    out = $${OUT_PWD}/translations/$${PREFIX}-$$1

    s = $$replace(1,-,_)
    lupdate_target = lupdate_$$s
    lrelease_target = lrelease_$$s

    $${lupdate_target}.commands = lupdate -noobsolete $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = $${out}.qm
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += $${out}.qm
    qm.files += $${out}.qm

    export($${lupdate_target}.commands)
    export($${lrelease_target}.target)
    export($${lrelease_target}.depends)
    export($${lrelease_target}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(PRE_TARGETDEPS)
    export(qm.files)
}

LANGUAGES = fi fr hu nl pl ru sv es

addTrFile($${NAME})
for(l, LANGUAGES) {
    addTrFile($${NAME}-$$l)
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm

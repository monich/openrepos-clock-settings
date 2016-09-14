TEMPLATE = lib
TARGET = clocksettings

PREFIX = openrepos
NAME = $${PREFIX}-clock-settings
TARGETPATH = $$[QT_INSTALL_QML]/$${PREFIX}/clock/settings

QT += qml dbus
CONFIG += plugin
QMAKE_CXXFLAGS += -Wno-unused-parameter

import.files = src/qmldir
import.path = $$TARGETPATH
target.path = $$TARGETPATH

settings_qml.path = /usr/share/$${NAME}
settings_qml.files = qml/*.qml

settings_json.path = /usr/share/jolla-settings/entries
settings_json.files = $${NAME}.json

OTHER_FILES += \
  *.json \
  translations/*.ts \
  src/*.xml \
  qml/*.qml \
  rpm/*.spec

SOURCES += \
  src/clocksettings.cpp \
  src/clocksettingsplugin.cpp

HEADERS += \
  src/clocksettings.h \
  src/clocksettingsplugin.h

INSTALLS += target import settings_qml settings_json

# D-Bus interfaces
DBUS_INTERFACES += timed
timed.files = src/com.nokia.time.xml
timed.header_flags = -N -c TimeDaemon
timed.source_flags = -N -c TimeDaemon

# Translations
TRANSLATIONS_PATH = /usr/share/translations
TRANSLATION_SOURCES = $${_PRO_FILE_PWD_}/qml
TRANSLATION_FILES = \
  clock-settings-ru \
  clock-settings

for(t, TRANSLATION_FILES) {
    suffix = $$replace(t,-,_)
    in = $${_PRO_FILE_PWD_}/translations/openrepos-$${t}
    out = $${OUT_PWD}/translations/$${PREFIX}-$${t}

    lupdate_target = lupdate_$$suffix
    lrelease_target = lrelease_$$suffix

    $${lupdate_target}.commands = lupdate -noobsolete $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = \"$${out}.qm\"
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += \"$${out}.qm\"
    qm.files += \"$${out}.qm\"
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm

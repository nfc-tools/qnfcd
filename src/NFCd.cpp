#include <QCoreApplication>
#include <QtDBus>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include "NFCd.h"
#include "NfcDeviceManager.h"
#include "nfcdevicemanageradaptor.h"


int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  NfcDeviceManager *nfcDeviceManager = new NfcDeviceManager();

  new NfcDeviceManagerAdaptor(nfcDeviceManager);

  QDBusConnection connection = QDBusConnection::systemBus();

    bool ret = connection.registerService ( "org.nfc_tools.nfcd" );
    if ( !ret ) {
    qDebug() <<  connection.lastError().message();
    qFatal("Unable to register service on D-Bus");
    } else {
  }

    if ( !connection.registerObject ( "/nfcd", nfcDeviceManager ) ) {
        qFatal ( "Unable to register device manager on D-Bus." );
}

    return app.exec();
}
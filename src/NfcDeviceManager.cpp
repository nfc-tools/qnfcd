#include "NfcDeviceManager.h"

#include <nfc/nfc-types.h>

#include "NfcDevice.h"
#include "nfcdeviceadaptor.h"

#define MAX_NFC_INITIATOR_COUNT 16

static nfc_connstring known_devices_desc[MAX_NFC_INITIATOR_COUNT];

NfcDeviceManager::NfcDeviceManager()
{
  startTimer(1000);
  _accessLock = new QMutex(QMutex::Recursive);
}

QStringList NfcDeviceManager::getDeviceList()
{
  QStringList devicesName;
  for(int i=0; i<_devices.size(); i++) {
    devicesName << _devices.at(i)->getName();
  }
  return devicesName;
}

QDBusObjectPath NfcDeviceManager::getDevicePathByName(const QString& deviceName) {
  QDBusObjectPath devicePath;
  for(int i=0; i<_devices.size(); i++) {
    if(deviceName == _devices.at(i)->getName()) {
      devicePath = _devices.at(i)->getPath();
    }
  }
  return devicePath;
}

QDBusObjectPath NfcDeviceManager::getDevicePathById(const uchar id) {
  QDBusObjectPath devicePath;
  for(int i=0; i<_devices.size(); i++) {
    if(id  == _devices.at(i)->getId()) {
      devicePath = _devices.at(i)->getPath();
    }
  }
  return devicePath;
}

void NfcDeviceManager::checkAvailableDevices()
{
  _accessLock->lock();
  // A new array is not required at each call (hence the 'static')
  static nfc_connstring polled_devices_desc[MAX_NFC_INITIATOR_COUNT];
  size_t found = 0;

  found = nfc_list_devices (NULL, polled_devices_desc, MAX_NFC_INITIATOR_COUNT);

  /* Look for disapeared devices */
  for (size_t i = 0; i< MAX_NFC_INITIATOR_COUNT; i++) {
    if (known_devices_desc[i][0]) {
      bool still_here = false;
      for (size_t n = 0; n < found; n++) {
        if (0 == strcmp(known_devices_desc[i], polled_devices_desc[n]))
          still_here = true;
      }
      if (!still_here) {
        unregisterDevice (i, known_devices_desc[i]);
        known_devices_desc[i][0] = '\0';
      }
    }
  }

  /* Look for new devices */
  for (size_t i=0; i < found; i++) {
    bool already_known = false;
    int n;

    for (n=0; n<MAX_NFC_INITIATOR_COUNT; n++) {
      if (strcmp(polled_devices_desc[i], known_devices_desc[n]) == 0)
        already_known = true;
    }

    if (!already_known) {
      for (n=0; n < MAX_NFC_INITIATOR_COUNT; n++) {
        if (known_devices_desc[n][0] == '\0') {
          registerDevice(n, polled_devices_desc[i]);
          strncpy (known_devices_desc[n] , polled_devices_desc[i],1024);
          break;
        }
      }
      if (n == MAX_NFC_INITIATOR_COUNT) {
        qDebug ("MAX_NFC_INITIATOR_COUNT initiators reached.");
      }
    }
  }
  _accessLock->unlock();
}

void NfcDeviceManager::checkAvailableTargets()
{
  for(int i=0; i<_devices.size(); i++) {
    NfcDevice* nfcDevice = _devices.at(i);
    nfcDevice->checkAvailableTargets();
  }
}

void NfcDeviceManager::timerEvent(QTimerEvent *event)
{
  Q_UNUSED(event);
  checkAvailableDevices();
  checkAvailableTargets();
}

void NfcDeviceManager::registerDevice(uchar id, nfc_connstring device)
{
  QString deviceName = QString(device);
  qDebug() << "Register new device \"" << deviceName << "\" with ID: " << id << "...";
  NfcDevice* nfcDevice = new NfcDevice(id, device, _accessLock);
  _devices << nfcDevice;

  new NfcDeviceAdaptor(nfcDevice);

  QDBusConnection connection = QDBusConnection::systemBus();
  QString path = QString("/nfcd") + QString("/device") + nfcDevice->getUuid().toString().remove(QRegExp("[{}-]"));
  qDebug() << "Trying to register \"" << deviceName << "\" at path: \"" << path << "\".";
  if( connection.registerObject(path, nfcDevice) ) {
    qDebug() << "Device \"" << deviceName << "\" is D-Bus registred (" << path << ").";
    nfcDevice->setPath(QDBusObjectPath(path));
    emit devicePlugged(id, deviceName);
  } else {
    qDebug() << connection.lastError().message();
    qFatal("Unable to register a new device on D-Bus.");
  }
}

void NfcDeviceManager::unregisterDevice(uchar id, QString device)
{
  qDebug() << "Unregister device \"" << device << "\" with ID: " << id << "...";
  for(int i=0; i<_devices.size(); i++) {
    if(_devices.at(i)->getId() == id) {
      NfcDevice* nfcDevice = _devices.takeAt(i);
      QDBusConnection connection = QDBusConnection::systemBus();
      QString path = QString("/nfcd") + QString("/device_") + nfcDevice->getUuid().toString().remove(QRegExp("[{}-]"));
      connection.unregisterObject(path);
      nfcDevice->setPath(QDBusObjectPath());
      qDebug() << "Device \"" << device << "\" is D-Bus unregistred (" << path << ").";
      delete(nfcDevice);
      break;
    }
  }
  emit deviceUnplugged (id, device);
}

const QDBusObjectPath NfcDeviceManager::getDefaultDevicePath()
{
  qDebug() << "getDefaultDevicePath()";
  if (_devices.count()) {
    return _devices.at(0)->getPath();
  }
  return QDBusObjectPath();
}


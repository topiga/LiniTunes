#include "idevice.h"

iDevice::iDevice(char* tmp_udid, QObject *parent)
    : QObject{parent}
{
    idevice_t device;
    lockdownd_client_t client = NULL;
    char* device_name = NULL;
    char* udid = NULL;

    /* Try to connect to first USB device */
    if (idevice_new(&device, tmp_udid) != IDEVICE_E_SUCCESS) {
        qDebug("ERROR: No device found!");
        return;
    }

    // Create a lockdown client
    if (lockdownd_client_new_with_handshake(device, &client, "LiniTunes") != LOCKDOWN_E_SUCCESS) {
        qDebug("Failed to create lockdown client");
        return;
    }

    // Retrieve the device's name
    if (lockdownd_get_device_name(client, &device_name) != LOCKDOWN_E_SUCCESS) {
        qDebug("Failed to get device name");
        return;
    }

    // Attempt to retrieve the device's UDID.
    if (lockdownd_get_device_udid(client, &udid) != LOCKDOWN_E_SUCCESS) {
        qDebug("Failed to get device udid");
        return;
    }

    // Log the connected device's UDID and name.
    qDebug("Connected with UDID: %s", udid);
    qDebug("Device name: %s", device_name);

    // Store the device, client, UDID, and device name in the class instance.
    this->_device = device;
    this->_client = client;
    this->_udid = udid;
    this->_device_name = device_name;

    // Retrieve and log additional device information.
    _get_basic_info();
    this->device_connected = true;
}

// Retrieves basic information about the device and logs it.
void iDevice::_get_basic_info() {
    // The method is composed of several blocks, each attempting to retrieve a specific type of device information using lockdownd_get_value and logging the result.

    // Retrieve and log the device's product type.
    {
        plist_t tmp_product_type = NULL;
        if (lockdownd_get_value(_client, NULL, "ProductType", &tmp_product_type) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_product_type, &this->_product_type);
            qDebug("ProductType: %s", this->_product_type);
            QThread::msleep(100);
        } else {
            qDebug("Failed to get device product type");
        }
        plist_free(tmp_product_type);
    }
    // Retrieve and log the device's class.
    // The rest of the method follows a similar pattern for retrieving and logging:
    // - Device class
    // - Software version
    // - Serial number
    // - ECID (Exclusive Chip ID)
    // - IMEI (International Mobile Equipment Identity)
    // - Model number combined with region information
    // - Storage capacity
    // - Available storage
    // - Battery capacity
    // - Battery charging state
    // Each block checks for success and logs either the retrieved value or a failure message.
    {
        plist_t tmp_device_class = NULL;
        if (lockdownd_get_value(_client, NULL, "DeviceClass", &tmp_device_class) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_device_class, &this->_device_class);
            qDebug("DeviceClass: %s", this->_device_class);
        } else {
            qDebug("Failed to get device device class");
        }
        plist_free(tmp_device_class);
    }
    {
        plist_t tmp_version = NULL;
        if (lockdownd_get_value(_client, NULL, "HumanReadableProductVersionString", &tmp_version) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_version, &this->_software_version);
            qDebug("HumanReadableProductVersionString: %s", this->_software_version);
        } else {
            if (lockdownd_get_value(_client, NULL, "ProductVersion", &tmp_version) == LOCKDOWN_E_SUCCESS) {
                plist_get_string_val(tmp_version, &this->_software_version);
                qDebug("ProductVersion: %s", this->_software_version);
            } else {
                qDebug("Failed to get device software version");
            }
        }
        plist_free(tmp_version);
    }
    {
        plist_t tmp_serial = NULL;
        if (lockdownd_get_value(_client, NULL, "SerialNumber", &tmp_serial) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_serial, &this->_serial);
            qDebug("SerialNumber: %s", this->_serial);
        } else {
            qDebug("Failed to get device serial number");
        }
        plist_free(tmp_serial);
    }
    {
        plist_t tmp_ecid = NULL;
        if (lockdownd_get_value(_client, NULL, "UniqueChipID", &tmp_ecid) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_ecid, &this->_ecid);
            qDebug("UniqueChipID: %lu", this->_ecid);
        } else {
            qDebug("Failed to get device ECID");
        }
        plist_free(tmp_ecid);
    }
    {
        plist_t tmp_imei = NULL;
        if (lockdownd_get_value(_client, NULL, "InternationalMobileEquipmentIdentity", &tmp_imei) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_imei, &this->_imei);
            //plist_write_to_stream(tmp_imei, stdout, PLIST_FORMAT_LIMD, PLIST_OPT_NONE);
            qDebug("InternationalMobileEquipmentIdentity: %s", this->_imei);
        } else {
            qDebug("Failed to get device IMEI");
            this->_imei=NULL;
        }
        plist_free(tmp_imei);
    }
    {
        plist_t tmp_model = NULL;
        plist_t tmp_region = NULL;
        char* tmp_model_ch = NULL;
        char* tmp_region_ch = NULL;
        if ((lockdownd_get_value(_client, NULL, "ModelNumber", &tmp_model) == LOCKDOWN_E_SUCCESS) && (lockdownd_get_value(_client, NULL, "RegionInfo", &tmp_region) == LOCKDOWN_E_SUCCESS)) {
            plist_get_string_val(tmp_model, &tmp_model_ch);
            plist_get_string_val(tmp_region, &tmp_region_ch);
            _model = (char*) malloc(strlen(tmp_model_ch)+strlen(tmp_region_ch));
            strcpy(_model, tmp_model_ch);
            strcat(_model, tmp_region_ch);
            qDebug("Full Model number: %s", this->_model);
        } else {
            qDebug("Failed to get device model number");
        }
        plist_free(tmp_model);
        plist_free(tmp_region);
        free(tmp_model_ch);
        free(tmp_region_ch);
    }
    {
        plist_t tmp_storage_capacity = NULL;
        if (lockdownd_get_value(_client, "com.apple.disk_usage", "TotalDiskCapacity", &tmp_storage_capacity) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_storage_capacity, &this->_storage_capacity);
            qDebug("Device storage capacity: %lu", this->_storage_capacity);
        } else {
            qDebug("Failed to get device capacity");
        }
    }
    {
        plist_t tmp_storage_left = NULL;
        if (lockdownd_get_value(_client, "com.apple.disk_usage", "TotalDataAvailable", &tmp_storage_left) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_storage_left, &this->_storage_left);
            qDebug("Device storage left: %lu", this->_storage_left);
        } else {
            qDebug("Failed to get device left");
        }
    }
    {
        plist_t tmp_battery_capacity = NULL;
        if (lockdownd_get_value(_client, "com.apple.mobile.battery", "BatteryCurrentCapacity", &tmp_battery_capacity) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_battery_capacity, &this->_battery_capacity);
            qDebug("Device battery capacity: %lu", this->_battery_capacity);
        } else {
            qDebug("Failed to get device capacity");
        }
    }
    {
        plist_t tmp_battery_charging = NULL;
        uint8_t tmp_battery_charging_bool;
        if (lockdownd_get_value(_client, "com.apple.mobile.battery", "BatteryIsCharging", &tmp_battery_charging) == LOCKDOWN_E_SUCCESS) {
            plist_get_bool_val(tmp_battery_charging, &tmp_battery_charging_bool);
//            _battery_charging = (bool) tmp_battery_charging_bool;
            qDebug("Is device charging: %x", tmp_battery_charging_bool);
        } else {
            qDebug("Failed to get device capacity");
        }
    }
}

// Returns the storage capacity of the device in a human-readable format.
QString iDevice::storage_capacity() {
    if (_storage_capacity>=1000000000000) {
        return QString(QString::number(_storage_capacity%1000000000000)+"TB");
    }
    if (_storage_capacity>=1000000000) {
        return QString(QString::number(_storage_capacity/1000000000)+"GB");
    }
    if (_storage_capacity>=1000000) {
        return QString(QString::number(_storage_capacity/1000000)+"MB");
    }
    if (_storage_capacity>=1000) {
        return QString(QString::number(_storage_capacity/1000)+"kB");
    }
    return QString::number(_storage_capacity);
}

// Returns the available storage on the device in a human-readable format.
QString iDevice::storage_left() {
    if (_storage_left>=1000000000000) {
        return QString(QString::number(_storage_left/1000000000000)+" TB");
    }
    if (_storage_left>=1000000000) {
        return QString(QString::number(_storage_left/1000000000)+"."+QString::number((_storage_left/100000000)%10)+" GB");
    }
    if (_storage_left>=1000000) {
        return QString(QString::number(_storage_left/1000000)+"."+QString::number((_storage_left/100000)%10)+" MB");
    }
    if (_storage_left>=1000) {
        return QString(QString::number(_storage_left/1000)+"."+QString::number((_storage_left/100)%10)+" kB");
    }
    return QString::number(_storage_left);
}

QString iDevice::device_image()
{
    if (QFile::exists(":/images/devices/"+this->product_type()+".png")) {
        return QString("/images/devices/"+this->product_type()+".png");
    } else if (QFile::exists(":/images/devices/"+this->device_class()+"Generic.png")) {
        return QString("/images/devices/"+this->device_class()+"Generic.png");
    } else if (QFile::exists(":/images/devices/Generic.png")) {
        return QString("/images/devices/Generic.png");
    }
    // return QString("/images/iDevice/iDevice_light_90x90.png");
    return QString("/images/iphone.png");
}

// Destructor for the iDevice class. Frees up resources upon object destruction.
iDevice::~iDevice() {
    // Free the lockdownd client, device, and strings allocated for device information if they exist.
    qDebug("Called");
    if (this->_client) {
        if (lockdownd_client_free(this->_client) != LOCKDOWN_E_SUCCESS) {
            qDebug("Error! Client not freed");
        }
    }
    if (this->_device) {
        if (idevice_free(this->_device) != IDEVICE_E_SUCCESS) {
            qDebug("Error! Device not freed");
        }
    }
    if (this->_device_name) {
        free(this->_device_name);
    }
    if (this->_udid) {
        free(this->_udid);
    }
    if (this->_imei) {
        free(this->_imei);
    }
    if (this->_serial) {
        free(this->_serial);
        //printf("Serial freed\n");
    }
    if (this->_model) {
        free(this->_model);
        //printf("model freed\n");
    }
    if (this->_software_version) {
        free(this->_software_version);
        //printf("software version freed\n");
    }
    if (this->_product_type) {
        free(this->_product_type);
        //printf("product type freed\n");
    }
}

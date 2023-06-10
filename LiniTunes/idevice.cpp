#include "idevice.h"

iDevice::iDevice(QObject *parent)
    : QObject{parent}
{
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    char* udid = NULL;
    char* device_name = NULL;

    /* Try to connect to first USB device */
    if (idevice_new_with_options(&device, NULL, IDEVICE_LOOKUP_USBMUX) != IDEVICE_E_SUCCESS) {
        printf("ERROR: No device found!\n");
        return;
    }

    // Create a lockdown client
    if (lockdownd_client_new_with_handshake(device, &client, "Example") != LOCKDOWN_E_SUCCESS) {
        printf("Failed to create lockdown client\n");
        return;
    }

    // Get device udid
    if (lockdownd_get_device_udid(client, &udid) != LOCKDOWN_E_SUCCESS) {
        printf("Failed to get the device UDID.\n");
        return;
    }

    // Retrieve the device's name
    if (lockdownd_get_device_name(client, &device_name) != LOCKDOWN_E_SUCCESS) {
        printf("Failed to get device name\n");
        return;
    }

    printf("Connected with UDID: %s\n", udid);
    printf("Device name: %s\n", device_name);

    // Assign
    this->_device = device;
    this->_client = client;
    this->_udid = udid;
    this->_device_name = device_name;

    _get_basic_info();
}

void iDevice::_get_basic_info() {
    {
        plist_t tmp_producttype = NULL;
        if (lockdownd_get_value(_client, NULL, "ProductType", &tmp_producttype) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_producttype, &this->_product_type);
            printf("ProductType: %s\n", this->_product_type);
        } else {
            printf("Failed to get device product type\n");
        }
        plist_free(tmp_producttype);
    }
    {
        plist_t tmp_version = NULL;
        if (lockdownd_get_value(_client, NULL, "HumanReadableProductVersionString", &tmp_version) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_version, &this->_software_version);
            printf("HumanReadableProductVersionString: %s\n", this->_software_version);
        } else {
            printf("Failed to get device software version\n");
        }
        plist_free(tmp_version);
    }
    {
        plist_t tmp_serial = NULL;
        if (lockdownd_get_value(_client, NULL, "SerialNumber", &tmp_serial) == LOCKDOWN_E_SUCCESS) {
            plist_get_string_val(tmp_serial, &this->_serial);
            printf("SerialNumber: %s\n", this->_serial);
        } else {
            printf("Failed to get device serial number\n");
        }
        plist_free(tmp_serial);
    }
    {
        plist_t tmp_ecid = NULL;
        if (lockdownd_get_value(_client, NULL, "UniqueChipID", &tmp_ecid) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_ecid, &this->_ecid);
            printf("UniqueChipID: %lu\n", this->_ecid);
        } else {
            printf("Failed to get device ECID\n");
        }
        plist_free(tmp_ecid);
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
            printf("Full Model number: %s\n", this->_model);
        } else {
            printf("Failed to get device model number\n");
        }
        plist_free(tmp_model);
        plist_free(tmp_region);
        free(tmp_model_ch);
        free(tmp_region_ch);
    }
    {
        plist_t tmp_capacity = NULL;
        if (lockdownd_get_value(_client, "com.apple.disk_usage", "TotalDiskCapacity", &tmp_capacity) == LOCKDOWN_E_SUCCESS) {
            plist_get_uint_val(tmp_capacity, &this->_capacity);
            printf("Device storage capacity: %lu\n", this->_capacity);
        } else {
            printf("Failed to get device capacity\n");
        }
    }
}

QString iDevice::device_name() {
    return QString(_device_name);
}

iDevice::~iDevice() {
    if (this->_client) {
        if (lockdownd_client_free(this->_client) != LOCKDOWN_E_SUCCESS) {
            printf("Error! Client not freed\n");
        }
    }
    if (this->_device) {
        if (idevice_free(this->_device) != IDEVICE_E_SUCCESS) {
            printf("Error! Device not freed\n");
        }
    }
    if (this->_device_name) {
        free(this->_device_name);
        //printf("Name freed\n");
    }
    if (this->_udid) {
        free(this->_udid);
        //printf("Udid freed\n");
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

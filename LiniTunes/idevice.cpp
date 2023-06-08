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
    this->udid = udid;
    this->device_name = device_name;

    get_basic_info();
}

void iDevice::get_basic_info() {
    {
        plist_t tmp_producttype = NULL;
        if (lockdownd_get_value(_client, NULL, "ProductType", &tmp_producttype) != LOCKDOWN_E_SUCCESS) {
            printf("Failed to get device product type\n");
        } else {
            plist_get_string_val(tmp_producttype, &this->product_type);
            printf("ProductType: %s\n", this->product_type);
        }
        plist_free(tmp_producttype);
    }
    {
        plist_t tmp_version = NULL;
        if (lockdownd_get_value(_client, NULL, "HumanReadableProductVersionString", &tmp_version) != LOCKDOWN_E_SUCCESS) {
            printf("Failed to get device software version\n");
        } else {
            plist_get_string_val(tmp_version, &this->software_version);
            printf("HumanReadableProductVersionString: %s\n", this->software_version);
        }
        plist_free(tmp_version);
    }
    {
        plist_t tmp_serial = NULL;
        if (lockdownd_get_value(_client, NULL, "SerialNumber", &tmp_serial) != LOCKDOWN_E_SUCCESS) {
            printf("Failed to get device serial number\n");
        } else {
            plist_get_string_val(tmp_serial, &this->serial);
            printf("SerialNumber: %s\n", this->serial);
        }
        plist_free(tmp_serial);
    }
    {
        plist_t tmp_ecid = NULL;
        if (lockdownd_get_value(_client, NULL, "UniqueChipID", &tmp_ecid) != LOCKDOWN_E_SUCCESS) {
            printf("Failed to get device ECID\n");
        } else {
            plist_get_uint_val(tmp_ecid, &this->ecid);
            printf("UniqueChipID: %lu\n", this->ecid);
        }
        plist_free(tmp_ecid);
    }
    {
        plist_t tmp_model = NULL;
        if (lockdownd_get_value(_client, NULL, "ModelNumber", &tmp_model) != LOCKDOWN_E_SUCCESS) {
            printf("Failed to get device model number\n");
        } else {
            plist_get_string_val(tmp_model, &this->model);
            printf("ModelNumber: %s\n", this->model);
        }
        plist_free(tmp_model);
    }
}

iDevice::~iDevice() {
    if (this->_client) {
        lockdownd_client_free(this->_client);
        printf("Client freed\n");
    }
    if (this->_device) {
        idevice_free(this->_device);
        printf("Device freed\n");
    }
    if (this->device_name) {
        free(this->device_name);
        printf("Name freed\n");
    }
    if (this->udid) {
        free(this->udid);
        printf("Udid freed\n");
    }
    if (this->udid) {
        free(this->serial);
        printf("Serial freed\n");
    }
    if (this->model) {
        free(this->model);
        printf("model freed\n");
    }
    if (this->software_version) {
        free(this->software_version);
        printf("software version freed\n");
    }
    if (this->product_type) {
        free(this->product_type);
        printf("product type freed\n");
    }
}

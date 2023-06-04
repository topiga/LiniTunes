#include "idevice.h"

iDevice::iDevice(QObject *parent)
    : QObject{parent}
{
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    char* udid;
    char* device_name = NULL;

    /* Try to connect to first USB device */
    if (idevice_new_with_options(&device, NULL, IDEVICE_LOOKUP_USBMUX) != IDEVICE_E_SUCCESS) {
        printf("ERROR: No device found!\n");
        return;
    }

    /* Retrieve the udid of the connected device */
    if (idevice_get_udid(device, &udid) != IDEVICE_E_SUCCESS) {
        printf("ERROR: Unable to get the device UDID.\n");
        idevice_free(device);
        return;
    }

    // Create a lockdown client
    if (lockdownd_client_new_with_handshake(device, &client, "Example") != LOCKDOWN_E_SUCCESS) {
        printf("Failed to create lockdown client\n");
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
}

#include "idevice.h"

iDevice::iDevice(QObject *parent)
    : QObject{parent}
{
    /* Unique Device Identifier */
    static char *udid = NULL;

    /* Device Handle */
    idevice_t device = NULL;



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

    /* Outputs device identifier */
    printf("Connected with UDID: %s\n", udid);

    /* Cleanup */
    idevice_free(device);
    free(udid);
}

#include "protocol_device.h"

void protocol_device_register(protocol_device *dev, sendto_func send_to)
{
    if (dev) {
        dev->send_to = send_to;
    }
}

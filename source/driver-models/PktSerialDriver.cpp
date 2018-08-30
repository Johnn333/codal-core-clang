/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "PktSerialProtocol.h"
#include "CodalDmesg.h"
#include "codal_target_hal.h"

using namespace codal;

int PktSerialDriver::queueControlPacket()
{
    PKT_DMESG("QUEUED CP");
    ControlPacket cp;
    memset(&cp, target_random(256), sizeof(ControlPacket));

    cp.packet_type = CONTROL_PKT_TYPE_HELLO;
    cp.address = device.address;
    cp.flags = device.flags & 0x00FF;
    cp.driver_class = this->driver_class;
    cp.serial_number = device.serial_number;

    PktSerialProtocol::send((uint8_t *)&cp, sizeof(ControlPacket), 0);

    return DEVICE_OK;
}

PktSerialDriver::PktSerialDriver(PktDevice d, uint32_t driver_class, uint16_t id)
{
    memset((uint8_t*)&device, 0, sizeof(PktDevice));

    this->driver_class = driver_class;
    this->device = d;
    this->id = id;
}

bool PktSerialDriver::isConnected()
{
    return (this->device.flags & PKT_DEVICE_FLAGS_INITIALISED) ? true : false;
}

int PktSerialDriver::deviceConnected(PktDevice device)
{
    DMESG("CONNECTED a:%d sn:%d",device.address,device.serial_number);
    uint16_t flags = this->device.flags & 0xFF00;
    this->device = device;
    this->device.flags = (flags | PKT_DEVICE_FLAGS_INITIALISED | PKT_DEVICE_FLAGS_CP_SEEN);
    Event(this->id, PKT_DRIVER_EVT_CONNECTED);
    return DEVICE_OK;
}

int PktSerialDriver::deviceRemoved()
{
    DMESG("DISCONN a:%d sn:%d",device.address,device.serial_number);
    this->device.flags &= ~(PKT_DEVICE_FLAGS_INITIALISED);
    this->device.rolling_counter = 0;
    Event(this->id, PKT_DRIVER_EVT_DISCONNECTED);
    return DEVICE_OK;
}

PktSerialDriver::~PktSerialDriver()
{
    PktSerialProtocol::instance->remove(*this);
}
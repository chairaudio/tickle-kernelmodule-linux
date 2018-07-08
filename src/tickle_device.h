#pragma once

#include "./tickle.h"

typedef struct TickleDevice_ {
    TickleService* service;
    TickleDeviceContext* context;
    BigBuffer* buffer;
    SmallBuffer serial;
    SmallBuffer version;
} TickleDevice;

void tickle_device_init(TickleDevice*, TickleService*);
void tickle_device_exit(TickleDevice*);

bool tickle_device_is_connected(TickleDevice*);
void tickle_device_set_context(TickleDevice*, TickleDeviceContext*);
void tickle_device_copy_buffer(TickleDevice*, BigBuffer*);
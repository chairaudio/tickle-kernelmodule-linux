#pragma once

#include "./tickle_io.h"
#include "./tickle_client.h"
#include "./tickle_usb.h"
#include "./tickle_device.h"

typedef struct TickleService_ {
    TickleIO io;
    TickleClient client;
    tickleUSB usb;
    TickleDevice device;
} TickleService;

int tickle_service_init(TickleService*);
void tickle_service_exit(TickleService*);

TickleClient* tickle_service_create_client(TickleService*);
void tickle_service_on_client_ready(TickleService*, TickleClient*);
TickleDevice* tickle_service_create_device(TickleService*);
void tickle_service_release_device(TickleService*, TickleDevice*);
void tickle_service_on_device_ready(TickleService*, TickleDevice*);
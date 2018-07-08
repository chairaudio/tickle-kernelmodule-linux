#pragma once

#include "./tickle.h"

typedef struct tickleUSB_ {
    TickleService* service;
} tickleUSB;

typedef struct TickleDeviceContext_ {
    tickleUSB* usb;
    TickleDevice* device;
    struct usb_device* usb_device_;
    struct usb_interface* usb_interface_;

    struct urb* isoc_in_urb;
    BigBuffer* transfer_buffer;
} TickleDeviceContext;

int tickle_usb_init(tickleUSB*, TickleService*);
void tickle_usb_exit(tickleUSB*);
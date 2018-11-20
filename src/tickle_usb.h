#pragma once

#include "./tickle.h"

struct TickleUSB_ {
    TickleService* service;
};

struct TickleDeviceContext_ {
    TickleUSB* usb;
    TickleDevice* device;
    struct usb_device* usb_device_;
    struct usb_interface* usb_interface_;

    struct urb* isoc_in_urb;
    struct urb* int_out_urb;
    BigBuffer* transfer_buffer;
    MiniBuffer* int_out_buffer;
};

int tickle_usb_init(TickleUSB*, TickleService*);
void tickle_usb_exit(TickleUSB*);

void tickle_usb_send(TickleUSB*, TickleDeviceContext*, MiniBuffer*);

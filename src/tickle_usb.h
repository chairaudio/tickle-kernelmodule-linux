#pragma once

#include "./tickle.h"

struct TickleUSB_ {
    TickleService* service;
};

#define TICKLE_URB_POOL_SIZE 16

struct TickleUrb_ {
    int idx;
    BigBuffer* transfer_buffer;
    struct urb* isoc_in_urb;

    TickleUrbPool* pool;
};

struct TickleUrbPool_ {
    TickleUrb urbs[TICKLE_URB_POOL_SIZE];
    TickleDeviceContext* context;
};

struct TickleDeviceContext_ {
    TickleUSB* usb;
    TickleDevice* device;
    struct usb_device* usb_device_;
    struct usb_interface* usb_interface_;

    struct urb* isoc_in_urb;
    struct urb* int_out_urb;
    TickleUrbPool urb_pool;
    BigBuffer* transfer_buffer;
    MiniBuffer* int_out_buffer;
};

int tickle_usb_init(TickleUSB*, TickleService*);
void tickle_usb_exit(TickleUSB*);

void tickle_usb_send(TickleUSB*, TickleDeviceContext*, MiniBuffer*);

void tickle_urb_pool_init(TickleUrbPool*, TickleDeviceContext*);
void tickle_urb_init(TickleUrb*, TickleUrbPool*, uint16_t);
void tickle_urb_pool_submit(TickleUrbPool*);
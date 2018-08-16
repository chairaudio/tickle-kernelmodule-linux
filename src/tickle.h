#pragma once

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/usb.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define TICKLE "tickle"
#define TICKLE_VID 0x04b4
#define TICKLE_PID 0xbeef

typedef struct TickleService_ TickleService;
typedef struct TickleIO_ TickleIO;
typedef struct TickleClient_ TickleClient;
typedef struct tickleUSB_ tickleUSB;
typedef struct TickleDevice_ TickleDevice;
typedef struct TickleDeviceContext_ TickleDeviceContext;

#include "./tickle_protocol.h"
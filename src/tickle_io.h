#pragma once

#include <linux/types.h>
#include <linux/cdev.h>

#include "./tickle.h"

struct TickleIO_ {
    TickleService* service;
    dev_t dev;
    struct cdev cdev;
    struct class* cls;
    struct device* devnode;
};

int tickle_io_init(TickleIO*, TickleService*);
void tickle_io_exit(TickleIO*);

#pragma once

#include "./tickle.h"

struct TickleClient_ {
    TickleService* service;
    TickleIO* io;
    TickleDevice* device;
};

void tickle_client_init(TickleClient*, TickleService*);
void tickle_client_exit(TickleClient*);

bool tickle_client_is_connected(TickleClient*);
void tickle_client_set_io(TickleClient*, TickleIO*);
void tickle_client_set_device(TickleClient*, TickleDevice*);
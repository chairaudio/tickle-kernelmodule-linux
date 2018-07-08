#include "./tickle_client.h"
#include "./tickle_service.h"

void tickle_client_init(TickleClient* self, TickleService* service) {
    self->service = service;
    self->io = NULL;
    self->device = NULL;
}

void tickle_client_exit(TickleClient* self) {
    if (tickle_client_is_connected(self)) {
        // disconnect
    }
}

bool tickle_client_is_connected(TickleClient* self) {
    return self->io != NULL;
}

void tickle_client_set_io(TickleClient* self, TickleIO* io) {
    self->io = io;
    if (self->io) {
        tickle_service_on_client_ready(self->service, self);
    }
}

void tickle_client_set_device(TickleClient* self, TickleDevice* device) {
    self->device = device;
    /*
    if (self->device) {
        tickle_service_on_client_ready(self->service, self);
    } */
}
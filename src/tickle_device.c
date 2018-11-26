#include "./tickle_device.h"
#include "./tickle_service.h"
#include "./tickle_usb.h"

void tickle_device_init(TickleDevice* self, TickleService* service) {
    self->service = service;
    self->context = NULL;
    self->buffer = kmalloc(BIG_BUFFER_SIZE, GFP_KERNEL);
    self->serial.size = 0;
    self->version.size = 0;
    spin_lock_init(&self->frame_lock);
}

void tickle_device_exit(TickleDevice* self) {
    self->context = NULL;
}

bool tickle_device_is_connected(TickleDevice* self) {
    return self->context != NULL;
}

void tickle_device_set_context(TickleDevice* self,
                               TickleDeviceContext* context) {
    self->context = context;

    if (self->context) {
        tickle_service_on_device_ready(self->service, self);
    } else {
        self->serial.size = self->version.size = 0;
    }
}

void tickle_device_copy_buffer_in(TickleDevice* self, BigBuffer* buffer) {
    unsigned long flags;
    spin_lock_irqsave(&self->frame_lock, flags);
    memcpy(self->buffer, buffer, BIG_BUFFER_SIZE);
    spin_unlock_irqrestore(&self->frame_lock, flags);
}

void tickle_device_copy_buffer_out(TickleDevice* self, BigBuffer* buffer) {
    unsigned long flags;
    spin_lock_irqsave(&self->frame_lock, flags);
    memcpy(buffer, self->buffer, BIG_BUFFER_SIZE);
    spin_unlock_irqrestore(&self->frame_lock, flags);
}
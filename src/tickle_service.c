#include "./tickle_service.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

int tickle_service_init(TickleService* self) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    tickle_io_init(&self->io, self);
    tickle_client_init(&self->client, self);
    tickle_usb_init(&self->usb, self);
    tickle_device_init(&self->device, self);
    return 0;
}

void tickle_service_exit(TickleService* self) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    tickle_client_exit(&self->client);
    tickle_io_exit(&self->io);
    tickle_device_exit(&self->device);
    tickle_usb_exit(&self->usb);
}

TickleClient* tickle_service_create_client(TickleService* self) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    if (!tickle_client_is_connected(&self->client)) {
        return &self->client;
    }
    return NULL;
}

void tickle_service_on_client_ready(TickleService* self, TickleClient* client) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    /*
    if (tickle_device_is_connected(&self->device)) {
        tickle_client_set_device(client, &self->device);
    } */
    tickle_client_set_device(client, &self->device);
}

TickleDevice* tickle_service_create_device(TickleService* self) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    if (!tickle_device_is_connected(&self->device)) {
        return &self->device;
    }
    return NULL;
}

void tickle_service_release_device(TickleService* self, TickleDevice* device) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
}

void tickle_service_on_device_ready(TickleService* self, TickleDevice* device) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    /*
    if (tickle_client_is_connected(&self->client)) {
        tickle_client_set_device(&self->client, device);
    } */
}
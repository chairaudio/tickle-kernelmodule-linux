#include "./tickle_usb.h"
#include "./tickle_service.h"
#include "./tickle_device.h"

static int _probe(struct usb_interface*, const struct usb_device_id*);
static void _disconnect(struct usb_interface*);

// https://www.kernel.org/doc/html/v4.13/driver-api/usb/power-management.html
// https://www.kernel.org/doc/html/v4.13/driver-api/usb/usb.html#c.usb_driver
// https://www.kernel.org/doc/html/v4.13/driver-api/usb/persist.html#usb-persist
static int _suspend(struct usb_interface*, pm_message_t);
static int _resume(struct usb_interface*);
static int _reset_resume(struct usb_interface*);

static struct usb_device_id tickle_usb_device_ids[] = {
    {USB_DEVICE(TICKLE_VID, TICKLE_PID)},
    {}};
MODULE_DEVICE_TABLE(usb, tickle_usb_device_ids);

static struct usb_driver tickle_usb_driver = {
    .name = TICKLE,
    .probe = _probe,
    .disconnect = _disconnect,
    .suspend = _suspend,
    .resume = _resume,
    .reset_resume = _reset_resume,
    .id_table = tickle_usb_device_ids};

static uint64_t count = 0;
void _complete(struct urb* urb) {
    int result = 0;
    TickleDeviceContext* context = urb->context;

    if (urb->status) {
        printk(KERN_INFO "urb _complete status %d\n", urb->status);
        return;
    }

    // https://www.kernel.org/doc/html/v4.13/driver-api/usb/error-codes.html#error-codes-returned-by-in-urb-status-or-in-iso-frame-desc-n-status-for-iso
    // Completion handlers for isochronous URBs should only see
    // urb->status set to zero, -ENOENT, -ECONNRESET, -ESHUTDOWN, or
    // -EREMOTEIO.

    if ((count % 1000) == 0) {
        struct isoc_frame* frame = (struct isoc_frame*)context->transfer_buffer;
        printk(KERN_INFO "urb _complete status %d\n", urb->status);
        // -115 EINPROGRESS
        printk(KERN_INFO "urb _complete error_count %d\n", urb->error_count);
        printk(KERN_INFO "urb _complete iso_frame_desc status %d\n",
               urb->iso_frame_desc[0].status);
        // -18 : -EXDEV : ISO transfer only partially completed
        // -71 : -EPROTO

        printk(KERN_INFO
               "urb _complete iso_frame_desc actual_length %d startframe %d\n",
               urb->iso_frame_desc[0].actual_length, urb->start_frame);
        printk(KERN_INFO "frame->number %d\n", frame->number);
        printk(KERN_INFO "frame->n_samples %d\n", frame->n_samples);
    }

    tickle_device_copy_buffer(context->device, context->transfer_buffer);

    result = usb_submit_urb(context->isoc_in_urb, GFP_KERNEL);
    if (result) {
        printk(KERN_INFO "usb_submit_urb failed with result %d\n", result);
    }

    count++;
}

int _probe(struct usb_interface* interface, const struct usb_device_id* id) {
    int error;
    // int endpoint_index;

    printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);

    if (id->idProduct == TICKLE_PID) {
        // struct usb_host_interface* host_interface;
        TickleUSB* self = (TickleUSB*)id->driver_info;
        TickleDevice* device = tickle_service_create_device(self->service);
        TickleDeviceContext* device_context = NULL;
        struct usb_device* usb_device_ = interface_to_usbdev(interface);

        if (device == NULL) {
            return -1;
        }

        strcpy(device->serial.data, usb_device_->serial);
        device->serial.size = strlen(device->serial.data);
        snprintf(device->version.data, 16, "0.%d",
                 usb_device_->descriptor.bcdDevice);
        device->version.size = strlen(device->version.data);

        device_context = kmalloc(sizeof(TickleDeviceContext), GFP_KERNEL);
        device_context->usb = self;
        device_context->device = device;
        device_context->usb_interface_ = interface;
        device_context->usb_device_ = usb_device_;
        device_context->transfer_buffer = kmalloc(BIG_BUFFER_SIZE, GFP_KERNEL);
        device_context->int_out_buffer = kmalloc(MINI_BUFFER_SIZE, GFP_KERNEL);
        device_context->isoc_in_urb = usb_alloc_urb(1, GFP_KERNEL);
        device_context->isoc_in_urb->dev = device_context->usb_device_;
        device_context->isoc_in_urb->pipe =
            usb_rcvisocpipe(device_context->usb_device_, 0x83);
        device_context->isoc_in_urb->transfer_flags = URB_ISO_ASAP;
        device_context->isoc_in_urb->transfer_buffer =
            device_context->transfer_buffer;
        device_context->isoc_in_urb->transfer_buffer_length = BIG_BUFFER_SIZE;
        device_context->isoc_in_urb->context = device_context;
        device_context->isoc_in_urb->complete = _complete;
        device_context->isoc_in_urb->number_of_packets = 1;
        device_context->isoc_in_urb->interval = 1;
        {
            struct usb_iso_packet_descriptor* iso =
                &device_context->isoc_in_urb->iso_frame_desc[0];
            iso->offset = 0;
            iso->length = BIG_BUFFER_SIZE;
        }
        device_context->int_out_urb = usb_alloc_urb(0, GFP_KERNEL);
        tickle_urb_pool_init(&device_context->urb_pool, device_context);

        error = usb_set_interface(
            device_context->usb_device_,
            interface->altsetting[0].desc.bInterfaceNumber, 1);

        usb_set_intfdata(interface, device_context);
        // get the pipes
        // host_interface = interface->cur_altsetting;
        usb_submit_urb(device_context->isoc_in_urb, GFP_KERNEL);
        tickle_device_set_context(device, device_context);
        {  // start streaming
            /*
            int result =
                usb_submit_urb(device_context->isoc_in_urb, GFP_KERNEL);
            printk(KERN_INFO "stream start usb_submit_urb result %d\n", result);
            switch (result) {
                case -EBUSY:
                    printk(KERN_INFO "-EBUSY\n");
                    break;
                case -ENODEV:
                    printk(KERN_INFO "-ENODEV\n");
                    break;
                case 0:
                    printk(KERN_INFO "here we go\n");
                    break;
                default:
                    printk(KERN_INFO "dunno Y\n");
                    break;*/
        }
        // printk(KERN_INFO "take\n");
        return 0;  // we take the device
    }
    // printk(KERN_INFO "dont take\n");
    return -1;  // we do not take the device
}

void _disconnect(struct usb_interface* interface) {
    TickleDeviceContext* device_context = usb_get_intfdata(interface);
    printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    tickle_device_set_context(device_context->device, NULL);

    // TODO: free
    // usb_unlink_urb(device_context->isoc_in_urb);
    // kfree(device_context);
}

int _suspend(struct usb_interface* interface, pm_message_t message) {
    TickleDeviceContext* device_context = usb_get_intfdata(interface);
    printk(KERN_INFO "TICKLE: _suspend\n");
    usb_kill_urb(device_context->isoc_in_urb);
    // usb_unlink_urb(device_context->isoc_in_urb);
    tickle_device_set_context(device_context->device, NULL);
    return 0;
}

int _resume(struct usb_interface* interface) {
    printk(KERN_INFO "TICKLE: _resume\n");
    return 0;
}

int _reset_resume(struct usb_interface* interface) {
    printk(KERN_INFO "TICKLE: _reset_resume\n");
    return 0;
}

int tickle_usb_init(TickleUSB* self, TickleService* service) {
    int result;

    self->service = service;

    // register this driver with the USB subsystem
    tickle_usb_device_ids[0].driver_info = (kernel_ulong_t)self;
    result = usb_register(&tickle_usb_driver);
    if (result < 0) {
        printk(KERN_INFO "usb_register failed with error %d", result);
        return -1;  // Non-zero return means that the module couldn't be loaded.
    }

    return 0;
}

void tickle_usb_exit(TickleUSB* self) {
    usb_deregister(&tickle_usb_driver);
}

void tickle_usb_send(TickleUSB* self,
                     TickleDeviceContext* context,
                     MiniBuffer* buffer) {
    int result = 0, actual = 0;
    /*
    usb_fill_int_urb(context->int_out_urb,
        context->usb_device_, usb_sndintpipe(context->usb_device_, 0x02),
        buffer, 16,
        NULL, context->usb_device_, 10
    );
    result = usb_submit_urb(context->int_out_urb, GFP_KERNEL);*/
    memcpy(context->int_out_buffer, buffer, MINI_BUFFER_SIZE);
    result = usb_interrupt_msg(
        context->usb_device_, usb_sndintpipe(context->usb_device_, 0x02),
        context->int_out_buffer, MINI_BUFFER_SIZE, &actual, 0);
    printk(KERN_INFO "usb_submit_urb result %d actual %d\n", result, actual);
}

void tickle_urb_init(TickleUrb* self, TickleUrbPool* pool, uint16_t idx) {
    printk(KERN_INFO "init turb %d\n", idx);
    self->pool = pool;
    self->idx = idx;
}

void tickle_urb_pool_init(TickleUrbPool* self, TickleDeviceContext* context) {
    uint16_t idx;
    printk(KERN_INFO "init pool \n");
    self->context = context;

    for (idx = 0; idx < TICKLE_URB_POOL_SIZE; ++idx) {
        TickleUrb* turb = &self->urbs[idx];
        tickle_urb_init(turb, self, idx);
    }
}

void tickle_urb_pool_submit(TickleUrbPool* self) {
    printk(KERN_INFO "tickle_urb_pool_submit\n");
}
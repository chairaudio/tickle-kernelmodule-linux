#include "./tickle_usb.h"
#include "./tickle_service.h"
#include "./tickle_device.h"

static int _probe(struct usb_interface*, const struct usb_device_id*);
static void _disconnect(struct usb_interface*);

static struct usb_device_id tickle_usb_device_ids[] = {
    {USB_DEVICE(TICKLE_VID, TICKLE_PID)},
    {}};
MODULE_DEVICE_TABLE(usb, tickle_usb_device_ids);

static struct usb_driver tickle_usb_driver = {
    .name = TICKLE,
    .probe = _probe,
    .disconnect = _disconnect,
    .id_table = tickle_usb_device_ids};

static uint64_t count = 0;
void _complete(struct urb* urb_) {
    int result = 0;
    TickleDeviceContext* context = urb_->context;

    if (urb_->status) {
        printk(KERN_INFO "urb _complete status %d\n", urb_->status);
        return;
    }

    // https://www.kernel.org/doc/html/v4.13/driver-api/usb/error-codes.html#error-codes-returned-by-in-urb-status-or-in-iso-frame-desc-n-status-for-iso
    // Completion handlers for isochronous URBs should only see
    // urb->status set to zero, -ENOENT, -ECONNRESET, -ESHUTDOWN, or
    // -EREMOTEIO.
    /*
    if ((count % 1000) == 0) {
        struct isoc_frame* frame = (struct isoc_frame*)context->transfer_buffer;
        printk(KERN_INFO "urb _complete status %d\n", urb_->status);
        // -115 EINPROGRESS
        printk(KERN_INFO "urb _complete error_count %d\n", urb_->error_count);
        printk(KERN_INFO "urb _complete iso_frame_desc status %d\n",
               urb_->iso_frame_desc[0].status);
        // -18 : -EXDEV : ISO transfer only partially completed
        // -71 : -EPROTO

        printk(KERN_INFO "urb _complete iso_frame_desc actual_length % d\n ",
               urb_->iso_frame_desc[0].actual_length);
        printk(KERN_INFO "frame->number %d\n", frame->number);
        printk(KERN_INFO "frame->n_samples %d\n", frame->n_samples);
    }*/

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

    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);

    if (id->idProduct == TICKLE_PID) {
        // struct usb_host_interface* host_interface;
        tickleUSB* self = (tickleUSB*)id->driver_info;
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
        return 0;  // we take the device
    }
    return -1;  // we do not take the device
}

void _disconnect(struct usb_interface* interface) {
    TickleDeviceContext* device_context = usb_get_intfdata(interface);
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    tickle_device_set_context(device_context->device, NULL);

    // TODO: free
    // usb_unlink_urb(device_context->isoc_in_urb);
    // kfree(device_context);
}

int tickle_usb_init(tickleUSB* self, TickleService* service) {
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

void tickle_usb_exit(tickleUSB* self) {
    usb_deregister(&tickle_usb_driver);
}

void tickle_usb_send(tickleUSB* self, TickleDeviceContext* context, MiniBuffer* buffer)
{
    int result = 0, actual = 0;
    /*
    usb_fill_int_urb(context->int_out_urb,
        context->usb_device_, usb_sndintpipe(context->usb_device_, 0x02),
        buffer, 16, 
        NULL, context->usb_device_, 10
    );        
    result = usb_submit_urb(context->int_out_urb, GFP_KERNEL);*/
    memcpy(context->int_out_buffer, buffer, MINI_BUFFER_SIZE);
    result = usb_interrupt_msg(context->usb_device_,
                    usb_sndintpipe(context->usb_device_, 0x02),
                    context->int_out_buffer, MINI_BUFFER_SIZE, &actual, 0
                );    
    printk(KERN_INFO "usb_submit_urb result %d actual %d\n", result, actual);                      
}
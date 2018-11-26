#include "./tickle_io.h"
#include "./tickle_service.h"
#include "./tickle.h"
#include "./gitversion.h"

static int _tickle_io_open(struct inode*, struct file*);
static int _tickle_io_close(struct inode*, struct file*);
static ssize_t _tickle_io_read(struct file*, char*, size_t, loff_t*);
static long _tickle_io_ioctl(struct file*, unsigned int, unsigned long);
/*static unsigned int _tickle_io_poll(struct file*,
                                         struct poll_table_struct*);
*/

static struct file_operations fops = {.owner = THIS_MODULE,
                                      .open = _tickle_io_open,
                                      .read = _tickle_io_read,
                                      .unlocked_ioctl = _tickle_io_ioctl,
                                      // .poll = _tickle_io_poll,
                                      .release = _tickle_io_close};

int tickle_io_init(TickleIO* self, TickleService* service) {
    int result, major, minor;
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    self->service = service;

    result = alloc_chrdev_region(&self->dev, 0, 1, TICKLE);
    if (result) {
        printk(KERN_INFO "cannot allocate chrdev region\n");
        return result;
    }
    major = MAJOR(self->dev);
    minor = MINOR(self->dev);
    // printk(KERN_INFO "got major %d minor %d\n", major, minor);

    cdev_init(&self->cdev, &fops);
    self->cdev.owner = THIS_MODULE;
    self->cdev.ops = &fops;
    result = cdev_add(&self->cdev, self->dev, 1);
    if (result) {
        printk(KERN_INFO "cannot add cdev\n");
        return result;
    }

    // is represented as /sys/class/tickle
    self->cls = class_create(THIS_MODULE, TICKLE);
    if (IS_ERR(self->cls)) {
        // unregister_chrdev(io.major, TICKLE);
        printk(KERN_ALERT "Failed to register device class\n");
    } else {
        // printk(KERN_INFO "device class registered\n");
    }
    // create /dev/tickle
    self->devnode = device_create(self->cls, NULL, self->dev, NULL, TICKLE);
    if (IS_ERR(self->devnode)) {
        printk(KERN_ALERT "Failed to create device node\n");
    }
    return 0;
}

void tickle_io_exit(TickleIO* self) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    device_destroy(self->cls, self->dev);
    class_unregister(self->cls);
    class_destroy(self->cls);
    cdev_del(&self->cdev);
    unregister_chrdev_region(self->dev, 1);
}

int _tickle_io_open(struct inode* inode, struct file* filp) {
    TickleIO* self = container_of(inode->i_cdev, TickleIO, cdev);
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    TickleClient* client = tickle_service_create_client(self->service);

    if (client == NULL) {
        // printk(KERN_INFO "deny\n");
        return -1;
    }
    tickle_client_set_io(client, self);
    filp->private_data = client;
    // tickle_service_client_did_connect(self->service);
    return 0;
}

int _tickle_io_close(struct inode* inode, struct file* filp) {
    // TickleIO* self = container_of(inode->i_cdev, TickleIO, cdev);
    TickleClient* client = filp->private_data;
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    tickle_client_set_io(client, NULL);
    return 0;
}

ssize_t _tickle_io_read(struct file* filp,
                        char* dst,
                        size_t size,
                        loff_t* offset) {
    // TickleIO* self = filp->private_data;
    // char* message = "foobar\0";
    // printk(KERN_INFO "%s %ld\n", __PRETTY_FUNCTION__, size);
    // copy_to_user(dst, message, 6);
    return 0;
}

long _tickle_io_ioctl(struct file* filp,
                      unsigned int command,
                      unsigned long argp) {
    int err = 0;

    if (_IOC_DIR(command) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user*)argp, _IOC_SIZE(command));
    else if (_IOC_DIR(command) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user*)argp, _IOC_SIZE(command));
    if (err)
        return -EFAULT;

    // printk(KERN_INFO "%s %d\n", __PRETTY_FUNCTION__, command);

    switch (command) {
        case TICKLE_IOC_PING: {
            // printk(KERN_INFO "TICKLE_IOC_PING\n");
            return 0;
        } break;
        case TICKLE_IOC_GET_KERNEL_MODULE_VERSION: {
            SmallBuffer version;
            strcpy(version.data, GITVERSION);
            version.size = strlen(GITVERSION);
            if (copy_to_user((void __user*)argp, &version,
                             sizeof(SmallBuffer))) {
                return -EFAULT;
            }
            return 0;
        } break;
        case TICKLE_IOC_GET_DEVICE_VERSION: {
            TickleClient* client = filp->private_data;
            if (client->device) {
                if (copy_to_user((void __user*)argp, &client->device->version,
                                 sizeof(SmallBuffer))) {
                    return -EFAULT;
                }
                return 0;
            }
            return -ENOTTY;
        } break;
        case TICKLE_IOC_GET_DEVICE_SERIAL: {
            TickleClient* client = filp->private_data;
            if (client->device) {
                if (copy_to_user((void __user*)argp, &client->device->serial,
                                 sizeof(SmallBuffer))) {
                    return -EFAULT;
                }
                return 0;
            }
            return -ENOTTY;
        } break;
        case TICKLE_IOC_START: {
            printk(KERN_INFO "%s TICKLE_IOC_START\n", __PRETTY_FUNCTION__);
        } break;
        case TICKLE_IOC_STOP: {
            printk(KERN_INFO "%s TICKLE_IOC_STOP\n", __PRETTY_FUNCTION__);
        } break;
        case TICKLE_IOC_READ_FRAME: {
            TickleClient* client = filp->private_data;
            BigBuffer buffer;
            if (client->device && tickle_device_is_connected(client->device)) {
                tickle_device_copy_buffer_out(client->device, &buffer);
                if (copy_to_user((void __user*)argp, &buffer,
                                 sizeof(BigBuffer))) {
                    return -EFAULT;
                }
                return 0;
            }
            return -ENOTTY;
        } break;
        case TICKLE_IOC_SET_COLOR: {
            MiniBuffer mini_buffer;
            TickleClient* client = filp->private_data;
            uint32_t* index_ptr;
            uint32_t* color_ptr;
            // printk(KERN_INFO "TICKLE_IOC_SET_COLOR\n");
            if (copy_from_user(&mini_buffer, (void __user*)argp,
                               sizeof(MiniBuffer))) {
                return -EFAULT;
            }
            index_ptr = (uint32_t*)&mini_buffer.data[0];
            color_ptr = (uint32_t*)&mini_buffer.data[4];
            // printk(KERN_INFO "OK : %d %d\n", *index_ptr, *color_ptr);
            tickle_usb_send(NULL, client->device->context, &mini_buffer);
            return 0;
        } break;
        default:
            return -ENOTTY;
    }
    return 0;
}

/*
unsigned int _tickle_io_poll(struct file* filp,
                                  struct poll_table_struct* poll_table) {
    return 0;
}
*/
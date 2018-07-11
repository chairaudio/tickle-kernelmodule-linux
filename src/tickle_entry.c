#include "./tickle.h"
#include "./gitversion.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chair.audio");
MODULE_DESCRIPTION(TICKLE);
MODULE_VERSION(GITVERSION);

#include "./tickle_service.h"
static TickleService service;

static int __init tickle_init(void) {
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
    return tickle_service_init(&service);
}

static void __exit tickle_exit(void) {
    tickle_service_exit(&service);
    // printk(KERN_INFO "%s\n", __PRETTY_FUNCTION__);
}

module_init(tickle_init);
module_exit(tickle_exit);

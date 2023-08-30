#include <linux/module.h>

static int __init helloworld_init(void)
{
	pr_info("Hello world\n");
	return 0;
}

static void __exit helloworld_cleanup(void)
{
	pr_info("Goodbye world\n");
}

module_init(helloworld_init);
module_exit(helloworld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carl-Johan Landin");
MODULE_DESCRIPTION("A simple helloworld kernel module");
MODULE_INFO(board, "Beaglebone black REV AS");

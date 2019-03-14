#include <linux/init.h>
#include <linux/module.h>

/*
 * The venerable hello world, kernel style.
 *  Taken from ldd3/misc-modules/hello.c
 */

static int hello_init(void)
{
	pr_info("Hello, world\n");

	return 0;
}

static void hello_exit(void)
{
	pr_info("Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tobin C. Harding");
MODULE_DESCRIPTION("Hello world, kernel style.");

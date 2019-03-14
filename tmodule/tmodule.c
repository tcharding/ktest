
/*
 * Module used for random kernel testing.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>

static unsigned int total_tests;
static unsigned int failed_tests;

/* tc() - Individual test case */
static void tc(int foo) {
	total_tests++;

	if (foo)
		goto out;

out:
	failed_tests++;
}

/* test() - Module test hook */
static void test(void)
{
	tc(0);
}

static int tmodule_init(void)
{
	pr_info("loaded\n");
	test();

	if (failed_tests == 0)
		pr_info("all %u tests passed\n", total_tests);
	else
		pr_warn("failed %u out of %u tests\n", failed_tests, total_tests);

	return failed_tests ? -EINVAL : 0;

	return 0;
}
module_init(tmodule_init);

static void tmodule_exit(void)
{
	pr_info("removed\n");
}
module_exit(tmodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tobin C. Harding");

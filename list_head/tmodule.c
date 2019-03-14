
/*
 * Module used for testing list_head
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

static unsigned int total_tests;
static unsigned int failed_tests;

/* List of items */
static LIST_HEAD(ilist);
#define TMODULE_NITEMS 10

/* Basic list'able structure */
struct item {
	struct list_head list;
	int val;
};

static struct item *create_item(int x)
{
	struct item *item;

	item = kmalloc(sizeof(struct item), GFP_KERNEL);
	if (!item)
		return NULL;

	item->val = x;
	return item;
}

static void destroy_item(struct item *item)
{
	kfree(item);
}

static void destroy_ilist(void)
{
	struct item *cur, *tmp;

	list_for_each_entry_safe(cur, tmp, &ilist, list) {
		list_del(&cur->list);
		destroy_item(cur);
	}
}

static int init_ilist(void)
{
	struct item *cur;
	int index, i;

	for (i = TMODULE_NITEMS - 1; i >= 0; i--) {
		cur = create_item(i);
		list_add(&cur->list, &ilist);
	}

	/* Verify the order*/
	index = 0;
	list_for_each_entry(cur, &ilist, list) {
		if (cur->val != index) {
			pr_err("failed to verify list order at index: %d got: %d\n",
				index, cur->val);
			goto out;
		}
		index++;
	}

	return 0;
out:
	failed_tests++;
	destroy_ilist();
	return -1;
}

static int test_rotate(void) {
	struct item *ptr;
	int val;
	const int desired_front = 5; /* arbitrary value < TMODULE_NITEMS */

	total_tests++;

	/* Get an arbitrary entry to make front of list */
	list_for_each_entry(ptr, &ilist, list) {
		if (ptr->val == desired_front)
			break;
	}

	list_rotate_to_front(&ptr->list, &ilist);

	/* Verify rotation worked */
	val = desired_front;
	list_for_each_entry(ptr, &ilist, list) {
		pr_info("rotate verify: want: %d got: %d\n", val, ptr->val);
		if (ptr->val != val) {
			pr_err("rotate verify failed: want: %d got: %d\n",
				val, ptr->val);
			failed_tests++;
			return -1;
		}

		val++;
		if (val % TMODULE_NITEMS == 0)
			val = 0;
	}

	return 0;
}

/* test() - Test module hook. */
static void test(void)
{
	int ret;

	ret = init_ilist();
	if (ret)
		return;

	(void)test_rotate();
	destroy_ilist();
}

static int __init tmodule_init(void)
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

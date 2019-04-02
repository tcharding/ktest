
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

	INIT_LIST_HEAD(&item->list);
	item->val = x;
	return item;
}

static void drain_ilist(struct list_head *head)
{
	struct item *cur, *tmp;

	list_for_each_entry_safe(cur, tmp, head, list) {
		list_del(&cur->list);
		kfree(cur);
	}
	INIT_LIST_HEAD(head);
}

static int populate_ilist(struct list_head *head, int nr_items)
{
	struct item *cur;
	int index, i;

	if (nr_items < 1)
		return -EINVAL;

	for (i = nr_items - 1; i >= 0; i--) {
		cur = create_item(i);
		if (!cur)
			goto out;
		list_add(&cur->list, head);
	}

	/* Verify the order*/
	index = 0;
	list_for_each_entry(cur, head, list) {
		if (cur->val != index) {
			pr_err("failed to verify list order at index: %d got: %d\n",
				index, cur->val);
			goto out;
		}
		index++;
	}

	return 0;
out:
	drain_ilist(head);
	return -1;
}

static int test_rotate_single(void)
{
	LIST_HEAD(ilist);
	struct item *ptr;
	int val = 10;		/* Arbitrary value */

	ptr = create_item(val);
	if (!ptr)
		return -ENOMEM;

	list_add(&ptr->list, &ilist);

	list_rotate_to_front(&ptr->list, &ilist);

	return 0;
}

static int test_rotate_many(void)
{
	LIST_HEAD(ilist);
	struct item *ptr;
	int desired_front;
	int val;
	int ret;

	populate_ilist(&ilist, 10);
	desired_front = 5;	/* Arbitrary value < 10 */

	/* Get an arbitrary entry to make front of list */
	list_for_each_entry(ptr, &ilist, list) {
		if (ptr->val == desired_front)
			break;
	}

	list_rotate_to_front(&ptr->list, &ilist);

	/* Verify rotation worked */
	val = desired_front;
	list_for_each_entry(ptr, &ilist, list) {
		if (ptr->val != val) {
			pr_err("rotate verify failed: want: %d got: %d\n",
				val, ptr->val);
			ret = -1;
			goto out;
		}

		val++;
		if (val % 10 == 0)
			val = 0;
	}

	ret = 0;
out:
	drain_ilist(&ilist);
	return ret;
}

/* do_test() - Do a unit test */
static void do_test(int (*fn)(void)) {
	int ret;

	total_tests++;
	ret = fn();
	if (ret)
		failed_tests++;
}

/* test() - Test module hook. */
static void test(void)
{
	do_test(test_rotate_single);
	do_test(test_rotate_many);
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

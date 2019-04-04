
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

/* Rotate the second of two items to the front. */
static int test_rotate_two(void)
{
	LIST_HEAD(ilist);
	struct item *first, *second;

	first = create_item(1);
	if (!first)
		return -ENOMEM;

	second = create_item(2);
	if (!second)
		return -ENOMEM;

	list_add(&first->list, &ilist);
	list_add_tail(&second->list, &ilist);

	list_rotate_to_front(&second->list, &ilist);

	return 0;
}

/* Rotate with 3 items. */
static int test_rotate_three(void)
{
	LIST_HEAD(ilist);
	struct item *first, *second, *third;

	first = create_item(1);
	if (!first)
		return -ENOMEM;

	second = create_item(2);
	if (!second)
		return -ENOMEM;

	third = create_item(3);
	if (!third)
		return -ENOMEM;

	list_add(&third->list, &ilist);
	list_add(&second->list, &ilist);
	list_add(&first->list, &ilist);

	list_rotate_to_front(&second->list, &ilist); /* order: 2, 1, 3 */
	list_rotate_to_front(&third->list, &ilist); /* order: 3, 2, 1 */

	return 0;
}

/* Rotate with 5 items. */
static int test_rotate_five(void)
{
	LIST_HEAD(ilist);
	struct item *first, *second, *third, *fourth, *fifth;

	first = create_item(1);
	second = create_item(2);
	third = create_item(3);
	fourth = create_item(3);
	fifth = create_item(3);
	if (!first || !second || !third)
		return -ENOMEM;

	list_add(&fifth->list, &ilist);
	list_add(&fourth->list, &ilist);
	list_add(&third->list, &ilist);
	list_add(&second->list, &ilist);
	list_add(&first->list, &ilist);

	/* order: 1 2 3 4 5 */

	list_rotate_to_front(&fourth->list, &ilist); /* order: 4 1 2 3 5 */
	list_rotate_to_front(&first->list, &ilist); /* order: 1 4 2 3 5 */
	list_rotate_to_front(&fifth->list, &ilist); /* order: 5 1 4 2 3 */

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

#ifdef FORCE_ERROR
/*
 * All unit tests above fail to trigger an error?
 * Smash the list on purpose.
 */
static int smash_the_list(void)
{
	LIST_HEAD(ilist);
	struct item *first, *second, *third;

	first = create_item(1);
	second = create_item(2);
	third = create_item(3);
	if (!first || !second || !third)
		return -ENOMEM;

	list_add(&ilist, &second->list);
	list_add(&ilist, &first->list);

	__list_add(&third->list, &ilist, &second->list);

	return 0;
}
#endif

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
	do_test(test_rotate_many);

	do_test(test_rotate_single);
	do_test(test_rotate_two);
	do_test(test_rotate_three);
	do_test(test_rotate_five);

#ifdef FORCE_ERROR
	/* This force triggers the DEBUG_LIST errors */
	smash_the_list();
#endif
}

/* verify_list_debug()- Verify that __list_add_valid emits warnings. */
static void verify_list_debug(void)
{
	LIST_HEAD(new);
	LIST_HEAD(prev);
	LIST_HEAD(next);

	__list_add_valid(&new, &prev, &next);
}

#define VERIFY_LIST_DEBUG 0
#define FORCE_ERROR 0
static int __init tmodule_init(void)
{
	pr_info("loaded\n");

	if (VERIFY_LIST_DEBUG)
		verify_list_debug();

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

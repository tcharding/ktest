
/*
 * Module used for testing slab allocators.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/printk.h>

static unsigned int total_tests;
static unsigned int failed_tests;

#define BASIC_BUF_SIZE 56	/* Control the size of the cached object */
struct basic {
	char buf[56];
	struct list_head list;
};

static void basic_ctor(void *ptr) {
	struct basic *b = ptr;

	memset(b->buf, 0, sizeof(b->buf));
	INIT_LIST_HEAD(&b->list);
};

static int generic_basic_test(void)
{
	struct kmem_cache *cachep;
	LIST_HEAD(blist);
	int nr_alloc = 1000;	/* Number of allocations to do */
	int keep = 3;		/* Keep 1 in 'keep' objects */
	int i;

	/* Should we use __alignof__(struct basic) here? */
	cachep = kmem_cache_create("basic", sizeof(struct basic), 0, 0, basic_ctor);
	if (!cachep) {
		pr_err("kmem_cache_create failed\n");
		return -1;
	}

	for (i = 0; i < nr_alloc; i++) {
		struct basic *b;

		b = kmem_cache_alloc(cachep, GFP_KERNEL);
		if (!b) {
			pr_err("kmem_cache_alloc failed\n");
			return -ENOMEM;
		}

		if (i % keep == 0) {
			list_add(&b->list, &blist);
			continue;
		}

		kmem_cache_free(cachep, b);
	}

	{
		struct basic *cur, *tmp;
		list_for_each_entry_safe(cur, tmp, &blist, list) {
			list_del(&cur->list);
			kmem_cache_free(cachep, cur);
		}
		kmem_cache_destroy(cachep);
	}
	return 0;
}

/* test_slab_generic() - Tests that any slab allocator should pass */
static void do_test(int (*fn)(void)) {
	int ret;

	total_tests++;
	ret = fn();
	if (ret)
		failed_tests++;
}

/* test() - Module test hook */
static void test(void)
{
	/* allocator generic tests */
	do_test(generic_basic_test);
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
MODULE_DESCRIPTION("Test module for the slab allocators");


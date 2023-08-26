#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 64

static WP wp_pool[NR_WP];
static WP *head, *free_;

static uint32_t size_s = 0;
uint32_t size()
{
	return size_s;
}
void init_wp_pool()
{
	int i;
	for (i = 0; i < NR_WP; i++)
	{
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
	if (!free_)
		assert(0);
	WP *p = free_;
	free_ = free_->next;
	return p;
}
void free_wp(WP *wp)
{
	WP *p = head;
	while (p && p != wp && p->next != wp)
		p = p->next;
	if (!p)
		assert(0);
	p->next = wp->next;
	/*clear things of wp*/;
	wp->next = free_;
	free_ = wp;
}
/*bool insert_wp(char *args)
{
	//WP *p=new_wp();
	size_s++;



}*/

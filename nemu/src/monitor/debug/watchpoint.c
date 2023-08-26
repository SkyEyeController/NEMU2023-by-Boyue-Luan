#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 64

static WP wp_pool[NR_WP];
static WP *head, *free_, *tail;
bool check_wp_isempty()
{
	return !size_s;
}
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

static WP *new_wp()
{
	if (!free_)
		assert(0);
	WP *p = free_;
	free_ = free_->next;
	return p;
}
static void free_wp(WP *wp)
{
	WP *p = head;
	while (p && p->next && p != wp && p->next != wp)
		p = p->next;
	if (!p||!p->next)
		{panic("NULL Point ERROR at free_wp function");}
	p->next = wp->next;
	/*clear things of wp*/;
	wp->next = free_;
	free_ = wp;
}
void insert_wp(char *args)
{
	WP *p = new_wp();
	size_s++;
	p->NO = size_s;
	p->args = args;
	p->next = head;
	head = p;
	while (p&&p->next)
	{
		p->next->NO = p->NO - 1;
		p = p->next;
	}
	printf("Watchpoint%d : %s was setted correctly\n", head->NO, head->args);
	return;
}
void print_watchpoints()
{
	WP *p = head;
	while (p)
	{
		printf("Watchpoint%d : %s\n", p->NO, p->args);
		p = p->next;
	}
	return;
}
void delete_wp(int n)
{
	WP *p = head, *tar = NULL;
	while (p)
	{
		if (p->NO == n)
		{
			free_wp(p);
		}
		p=p->next;
	}
	p=head;
	size_s--;
	head->NO=size_s;
	while (p&&p->next)
	{
		p->next->NO = p->NO - 1;
		p = p->next;
	}
	return;
}

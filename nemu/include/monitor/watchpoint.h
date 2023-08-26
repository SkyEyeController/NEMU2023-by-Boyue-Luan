#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	char *args;//表达式
	struct watchpoint *next;
	struct watchpoint *before;
	/* TODO: Add more members if necessary */


} WP;

#endif

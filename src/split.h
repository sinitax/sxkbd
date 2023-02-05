#pragma once

#define LEFT 0
#define RIGHT 1

#define SLAVE 0
#define MASTER 1

#define SPLIT_OPP(x) ((x) == LEFT ? RIGHT : LEFT)

void split_init(void);
void split_task(void);

void split_warn_master(const char *msg);

extern int split_role;

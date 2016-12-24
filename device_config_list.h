#ifndef DEVICE_CONFIG_LIST_H
#define DEVICE_CONFIG_LIST_H

#include "mytypes.h"

extern void add_item(struct Device *root, struct Device *newItem);
extern struct Device *get_item(struct Device *root, int id);
extern void free_list(struct Device *root);

#endif

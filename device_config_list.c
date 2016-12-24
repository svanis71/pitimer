#include <stdlib.h>
#include "mytypes.h"
#include "device_config_list.h"

void add_item(struct Device *root, struct Device *newItem)
{
	struct Device *list = root;
	while(list->next != NULL)
		list = list->next;
	newItem->next = NULL;
	list->next = newItem;
}

struct Device *get_item(struct Device *root, int id)
{
  struct Device *list = root;
  while(list != NULL) {
    if(list->id == id)
      break;
    list = list->next;
  }
  return list;
}

void free_list(struct Device *list)
{
	while(list->next != NULL) {
		struct Device *tail = list->next;
		free(list);
		list = tail; 
	}
	/* list will be the last item in the list */
	free(list); 
}

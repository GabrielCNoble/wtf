#ifndef LIST_H
#define LIST_H



struct list_t
{
	int element_size;
	int element_count;
	int max_elements;
	void *elements;

	//void (*dispose_callback)(void *element);
};

#ifdef __cplusplus
extern "C"
{
#endif

struct list_t list_create(int element_size, int max_elements, void (*dispose_callback)(void *element));

void list_destroy(struct list_t *list);

int list_add(struct list_t *list, void *data);

void list_remove(struct list_t *list, int index);

void *list_get(struct list_t *list, int index);

int list_get_count(struct list_t *list);

void list_resize(struct list_t *list, int new_size);

#ifdef __cplusplus
}
#endif

#endif

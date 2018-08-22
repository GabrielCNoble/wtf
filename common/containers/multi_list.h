#ifndef MULTI_LIST_H
#define MULTI_LIST_H


struct list_data_t
{
    int element_size;
    void *data;
};


struct multi_list_t
{
	int list_count;
    struct list_data_t *lists;

    int max_elements;
    int element_count;
};



struct multi_list_t multi_list_create(int initial_size, int list_count, ...);

void multi_list_destroy(struct multi_list_t *list);

void multi_list_advance(struct multi_list_t *list);

void multi_list_recede(struct multi_list_t *list);

int multi_list_add(struct multi_list_t *list, int list_index, void *data);

void multi_list_resize(struct multi_list_t *list, int increment);


#endif // MULTI_LIST_H

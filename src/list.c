#include <err.h>
#include <stdlib.h>

#include "list.h"
#include "common.h"

list_t* list_init() {
    list_t* list = (list_t*) malloc_or_die(sizeof(list_t));
    list->head = list_new(0);
    list->tail = list->head;
    list->head->id = 0;
    list->size = 0;
    return list;
}

void list_free(list_t* list) {
    if (!list) return;
    while (list_dequeue(list));

    assert(list->head == list->tail);
    free(list->head);
    free(list);
}

int list_dequeue(list_t* list) {
    assert(list);

    list_item_t* head = ATOMIC_READ(list->head);
    list_item_t* next = ATOMIC_READ(head->next);
    if (!next) return 0;

    ATOMIC_CMPXCHG(list->head, head, next);
    ATOMIC_DECREMENT(list->size, 1);

    free(head);
    return 1;
}

list_item_t* list_head(list_t* list) {
    assert(list);
    return ATOMIC_READ(list->head);
}

list_item_t* list_tail(list_t* list) {
    assert(list);
    return ATOMIC_READ(list->tail);
}

list_item_t* list_new(uint32_t size) {
    list_item_t* item = (list_item_t*) malloc_or_die(sizeof(list_item_t) + size);
    item->size = size;
    item->next = NULL;
    item->id = (uint64_t) -1;
    return item;
}

list_item_t* list_enqueue(list_t* list, list_item_t* item) {
    assert(list);
    assert(item);

    list_item_t* tail = ATOMIC_READ(list->tail);
    list_item_t* next = ATOMIC_READ(tail->next);
    item->id = tail->id + 1;

    ATOMIC_CMPXCHG(tail->next, next, item);
    ATOMIC_CMPXCHG(list->tail, tail, item);
    ATOMIC_INCREMENT(list->size, 1);
    return item;
}

list_item_t* list_enqueue_new(list_t* list, uint32_t size) {
    return list_enqueue(list, list_new(size));
}

size_t list_size(list_t* list) {
    assert(list);
    return ATOMIC_READ(list->size);
}

size_t list_distance(list_t* list, list_item_t* item) {
    assert(list);
    list_item_t* tail = ATOMIC_READ(list->tail);
    return ATOMIC_READ(tail->id) - ATOMIC_READ(item->id);
}

uint64_t list_item_id(list_item_t* item) {
    assert(item);
    return ATOMIC_READ(item->id);
}

list_item_t* list_item_next(list_item_t* item) {
    assert(item);
    return ATOMIC_READ(item->next);
}

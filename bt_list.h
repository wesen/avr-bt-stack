#ifndef BT_LIST_H__
#define BT_LIST_H__

/** list structure */
typedef struct bt_list_s {
   void             *data;
   struct bt_list_s *next;
} bt_list_t;

/** list head */
typedef struct bt_list_head_s {
   struct bt_list_s *list;
   struct bt_list_s *end;
   unsigned int     num;
} bt_list_head_t;

#define BT_LIST_OP(f) (void (*)(void *))(f)
#define BT_LIST_OP2(f) (void (*)(void *, void *))(f)
#define BT_LIST_CMP(f) (int (*)(void *, void *))(f)

void bt_list_init(bt_list_head_t *head);

bt_list_t *bt_list_new_with_data(int size);
void bt_list_free(bt_list_t *list);
void bt_list_delete(bt_list_t *list,
      void (*bt_free)(void *));
void bt_list_destroy(bt_list_head_t *head,
      void (*bt_free)(void *data));
int bt_list_free_from_data(bt_list_head_t *head, void *data);

int bt_list_insl_end(bt_list_head_t *head, bt_list_t *list);
int bt_list_insl_front(bt_list_head_t *head, bt_list_t *list);

bt_list_t *bt_list_getl(bt_list_head_t *head, bt_list_t *list);
void *bt_list_get(bt_list_head_t *head, bt_list_t *list);
bt_list_t *bt_list_getl_end(bt_list_head_t *head);
bt_list_t *bt_list_getl_front(bt_list_head_t *head);
void *bt_list_get_end(bt_list_head_t *head);
void *bt_list_get_front(bt_list_head_t *head);

bt_list_t *bt_list_search(bt_list_head_t *head,
      void *data,
      int (*cmp)(void *data, void *data2));
bt_list_t *bt_list_get_search(bt_list_head_t *head,
                              void *data,
                              int (*cmp)(void *data, void *data2));
void *bt_list_search_data(bt_list_head_t *head,
                          void *data,
                          int (*cmp)(void *data, void *data2));

void bt_list_map(bt_list_head_t *head, 
                 void (*f)(void *data));
void bt_list_map2(bt_list_head_t *head, 
                  void *ptr, void (*f)(void *ptr, void *data));

#define bt_list_push(h, x) bt_list_ins_front((h), (x))
#define bt_list_pop(h) bt_list_get_front((h))

#endif /* BT_LIST_H__ */

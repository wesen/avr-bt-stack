#include "bt_stack.h"
#ifdef DEBUG
#include <stdio.h>
#endif

void bt_list_init(bt_list_head_t *head) {
   if (!head)
      return;

   head->num = 0;
   head->list = head->end = NULL;
}

bt_list_t *bt_list_new_with_data(int size) {
   bt_list_t *list = NULL;
   
   list = malloc(sizeof(bt_list_t) + size);
   if (list) {
      list->next = NULL;
      list->data = ((unsigned char *)list) + sizeof(bt_list_t);
   }

   return list;
}

void bt_list_free(bt_list_t *list) {
   free(list);
}

void bt_list_delete(bt_list_t *list,
      void (*bt_free)(void *data)) {
   if (!list)
      return;

   if (list->data)
      bt_free(list->data);

   bt_list_free(list);
}

void bt_list_destroy(bt_list_head_t *head,
      void (*bt_free)(void *data)) {
   bt_list_t *list, *next;

   if (!head)
      return;

   head->num = 0;
   head->end = NULL;

   if (!(next = head->list))
      return;

   while (next) {
      list = next;
      next = list->next;
      bt_list_delete(list, bt_free);
   }
}

int bt_list_free_from_data(bt_list_head_t *head, void *data) {
   bt_list_t *list = (bt_list_t *)((unsigned char *)data -
                                   sizeof(bt_list_t));


   if (!bt_list_getl(head, list)) {
      PERR_STR("could not get list");
      free(list);
      return 0;
   }

   free(list);
   return 1;
}

int bt_list_insl_end(bt_list_head_t *head,
      bt_list_t *list) {
   bt_list_t *tmp;

   if (!head || !list)
      return 0;

   if (!head->list) {
      head->list = list;
      head->end  = list;
      head->num  = 1;
   } else {
      tmp  = head->list;
      while (tmp->next) {
         tmp = tmp->next;
      }
      
      tmp->next = list;
      head->end = list;
      head->num++;
   }

   return 1;
}

int bt_list_insl_front(bt_list_head_t *head,
      bt_list_t *list) {
   if (!head || !list)
      return 0;

   if (!head->list) {
      head->list = list;
      head->end  = list;
      head->num  = 1;
   } else {
      list->next = head->list;
      head->list = list;
      head->num++;
   }

   return 1;
}

bt_list_t *bt_list_getl(bt_list_head_t *head, bt_list_t *list) {
   bt_list_t *tmp = head->list;

   if (!head || !list || !tmp)
      return NULL;

   if (head->list == list) {
      head->list = list->next;
      if (head->end == list) 
         head->end = list->next;
      head->num--;

      return list;
   }

   while (tmp->next && (tmp->next != list)) {
      tmp = tmp->next;
   }

   if (!tmp->next)
      return NULL;
   else {
      tmp->next = list->next;
      if (head->end == list)
         head->end = list->next;
      head->num--;

      return list;
   }
}

void *bt_list_get(bt_list_head_t *head, bt_list_t *list) {
   bt_list_t *tmp = bt_list_getl(head, list);
   void      *data;

   if (!tmp)
      return NULL;
   else {
      data = tmp->data;
      return data;
   }
}

bt_list_t *bt_list_getl_end(bt_list_head_t *head) {
   return bt_list_getl(head, head->end);
}

bt_list_t *bt_list_getl_front(bt_list_head_t *head) {
   return bt_list_getl(head, head->list);
}

void *bt_list_get_end(bt_list_head_t *head) {
   return bt_list_get(head, head->end);
}

void *bt_list_get_front(bt_list_head_t *head) {
   return bt_list_get(head, head->list);
}

bt_list_t *bt_list_get_search(bt_list_head_t *head,
                              void *data,
                              int (*cmp)(void *data, void *data2)) {
   bt_list_t *list, *prev = NULL;

   if (!head || !head->list)
      return NULL;

   list = head->list;

   while (list) {
      if (cmp(data, list->data)) {
         if (!prev) {
            head->list = list->next;
         } else {
            prev->next = list->next;
         }

         head->num--;

         return list;
      }

      prev = list;
      list = list->next;
   }

   return NULL;
}

void *bt_list_search_data(bt_list_head_t *head,
                          void *data,
                          int (*cmp)(void *data, void *data2)) {
   bt_list_t *list = bt_list_search(head, data, cmp);

   if (!list)
      return NULL;
   else
      return list->data;
}

bt_list_t *bt_list_search(bt_list_head_t *head,
      void *data,
      int (*cmp)(void *data, void *data2)) {
   bt_list_t *list;

   if (!head || !head->list)
      return NULL;

   list = head->list;

   while (list) {
      if (cmp(data, list->data))
         return list;
      list = list->next;
   }

   return NULL;
}

void bt_list_map(bt_list_head_t *head, 
                 void (*f)(void *data)) {
   bt_list_t *list;

   if (!head || !head->list)
      return;

   list = head->list;

   while (list) {
      if (list->data)
         f(list->data);
      list = list->next;
   }
}

void bt_list_map2(bt_list_head_t *head, 
                  void *ptr, void (*f)(void *ptr, void *data)) {
   bt_list_t *list;

   if (!head || !head->list)
      return;

   list = head->list;

   while (list) {
      if (list->data)
         f(ptr, list->data);
      list = list->next;
   }
}

#ifdef BT_LIST_TEST
int bt_test_cmp(int *a, int *b) {
   if (*a == *b)
      return 1;

   return 0;
}

void bt_test_free(int *a) {
   free(a);
}

int main(void) {
   bt_list_head_t head;
   int *a, *b;

   bt_list_init(&head);
   a = malloc(sizeof(int));
   b = malloc(sizeof(int));
   *a = 5;
   *b = 6;

   if (bt_list_ins_end(&head, a) && bt_list_ins_front(&head, b))
      printf("Could insert\n");
   else
      return 0;

   bt_list_destroy(&head, BT_LIST_OP(bt_test_free));

   return 1;
}
#endif /* BT_LIST_TEST */

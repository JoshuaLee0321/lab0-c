#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* define macro */
#define q_insert_macro(type, func)                                \
    bool q_insert_##type(struct list_head *head, char *s)         \
    {                                                             \
        if (!head)                                                \
            return false;                                         \
        element_t *new_node = malloc(1 * sizeof(*new_node));      \
        if (!new_node)                                            \
            return false;                                         \
        new_node->value = malloc((strlen(s) + 1) * sizeof(char)); \
        if (!new_node->value) {                                   \
            q_release_element(new_node);                          \
            return false;                                         \
        }                                                         \
        memcpy(new_node->value, s, strlen(s) + 1);                \
        func(&new_node->list, head);                              \
        return true;                                              \
    }


#define q_remove_macro(type, func)                               \
    element_t *q_remove_##type(struct list_head *head, char *sp, \
                               size_t bufsize)                   \
    {                                                            \
        if (!head)                                               \
            return NULL;                                         \
        element_t *del = func(head, element_t, list);            \
        list_del(&del->list);                                    \
        if (sp != NULL) {                                        \
            strncpy(sp, del->value, bufsize - 1);                \
            sp[bufsize - 1] = '\0';                              \
        }                                                        \
        return del;                                              \
    }

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(1 * sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *it = NULL, *safe = NULL;
    list_for_each_entry_safe (it, safe, l, list) {
        q_release_element(it);
    }
    free(l);
    return;
}

/* Insert an element at head of queue */
q_insert_macro(head, list_add);

/* Insert an element at tail of queue */
q_insert_macro(tail, list_add_tail);

/* Remove an element from head of queue */
q_remove_macro(head, list_first_entry);

/* Remove an element from tail of queue */
q_remove_macro(tail, list_last_entry);

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *single_p, *double_p;
    single_p = double_p = head->next;
    while (double_p != (head) && double_p->next != (head)) {
        single_p = single_p->next;
        double_p = double_p->next->next;
    }
    element_t *del = list_entry(single_p, element_t, list);
    list_del(single_p);
    q_release_element(del);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // O(N) time and O(1) space
    if (!head || list_empty(head) || list_is_singular(head))
        return false;

    struct list_head *temp = head;
    while (temp->next != (head)) {
        element_t *prev = list_entry(temp, element_t, list);
        element_t *next = list_entry(temp->next, element_t, list);
        if (strcmp(prev->value, next->value) == 0) {
            list_del_init(&next->list);
            q_release_element(next);
        } else {
            temp = temp->next;
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || head->next->next == head)
        return;
    struct list_head *it = head->next;

    while (it != (head) && it->next != (head)) {
        list_move(it, it->next);
        it = it->next;
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *it, *safe = NULL;

    list_for_each_safe (it, safe, head) {
        list_move(it, head);
    }
    return;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head) || k < 1)
        return;
    int cnt = 0;
    struct list_head *cur, *tmp = NULL;
    struct list_head **indirect = &(head->next);
    struct list_head *safe = NULL;
    list_for_each_safe (cur, safe, head) {
        cnt++;  // add one element
        if (cnt % k == 0) {
            tmp = cur;
            while (*indirect != tmp) {
                list_move(tmp->prev, cur);
                cur = cur->next;
            }
            indirect = &(cur->next);
        }
    }

    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Merge two list into one queue */
struct list_head *merge_two_list(struct list_head *l1, struct list_head *l2)
{
    struct list_head *temp = NULL;
    struct list_head **indirect = &temp;
    for (struct list_head **node = NULL; l1 && l2; *node = (*node)->next) {
        element_t *e1 = list_entry(l1, element_t, list);
        element_t *e2 = list_entry(l2, element_t, list);
        if (strcmp(e1->value, e2->value) < 0)
            node = &l1;
        else
            node = &l2;
        *indirect = *node;
        indirect = &(*indirect)->next;
    }
    *indirect = (struct list_head *) ((u_int64_t) l1 | (u_int64_t) l2);
    return temp;
}

/* Divide the queue and sort every element */
struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    fast = slow;
    slow->prev->next = NULL;
    struct list_head *l1 = mergesort(head);
    struct list_head *l2 = mergesort(fast);
    return merge_two_list(l1, l2);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    head->prev->next = NULL;
    head->next = mergesort(head->next);
    struct list_head *current = head, *next = head->next;
    while (next) {
        next->prev = current;
        current = next;
        next = next->next;
    }
    current->next = head;
    head->prev = current;
}
/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    struct list_head *indirect = (head->prev);
    while (indirect->prev != (head)) {
        element_t *e1 = list_entry(indirect, element_t, list);
        element_t *e2 = list_entry(indirect->prev, element_t, list);
        if (strcmp(e1->value, e2->value) > 0) {
            list_del(indirect->prev);
            q_release_element(e2);
        } else {
            indirect = indirect->prev;
        }
    }
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending order */

int q_merge(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    else if (list_is_singular(head))
        return list_entry(head, queue_contex_t, chain)->size;

    queue_contex_t *nxt = list_entry(head->next, queue_contex_t, chain);
    queue_contex_t *cur = NULL;

    list_for_each_entry (cur, head, chain) {
        if (cur == nxt)
            continue;
        list_splice_init(cur->q, nxt->q);
        nxt->size = nxt->size + cur->size;
        cur->size = 0;
    }
    q_sort(nxt->q);
    return nxt->size;
    // https://leetcode.com/problems/merge-k-sorted-lists/
}

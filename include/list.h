#ifndef __DLIST_H
#define __DLIST_H

/* This file is from Linux Kernel (include/linux/list.h)
* and modified by simply removing hardware prefetching of list items.
* Here by copyright, credits attributed to wherever they belong.
* Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
*/

/*
* Simple doubly linked list implementation.
*
* Some of the internal functions (“__xxx”) are useful when
* manipulating whole lists rather than single entries, as
* sometimes we already know the next/prev entries and we can
* generate better code by using them directly rather than
* using the generic single-entry routines.
*/
/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );}
/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200)

//指针域部分，小结构体，用它来构建“纯粹链表”
struct list_head {
	struct list_head *next, *prev;
};

//节点结构体变量初始化
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
struct list_head name = LIST_HEAD_INIT(name)

//节点结构体指针初始化
#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
* Insert a new entry between two known consecutive entries.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/

//将new节点插入到prev和next节点之间
static inline void __list_add(struct list_head *new,
				struct list_head *prev,
				struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
* list_add – add a new entry
* @new: new entry to be added
* @head: list head to add it after
*
* Insert a new entry after the specified head.
* This is good for implementing stacks.
*/
//将new节点插入到head节点后面
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/**
* list_add_tail – add a new entry
* @new: new entry to be added
* @head: list head to add it before
*
* Insert a new entry before the specified head.
* This is useful for implementing queues.
*/
//将new节点插入到head的前面
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/*
* Delete a list entry by making the prev/next entries
* point to each other.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/
//取出prev和next节点之间的结点
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
* list_del – deletes entry from list.
* @entry: the element to delete from the list.
* Note: list_empty on entry does not return true after this, the entry is in an undefined state.
*/
//取出entry结点：让entry独立出来,前后指针域指向0地址
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/**
* list_del_init – deletes entry from list and reinitialize it.
* @entry: the element to delete from the list.
*/
//取出entry结点：让entry独立出来,前后指针域指向自己的地址
static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
* list_move – delete from one list and add as another’s head
* @list: the entry to move
* @head: the head that will precede our entry
*/
//将list移动到head后面
static inline void list_move(struct list_head *list,
				struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/**
* list_move_tail – delete from one list and add as another’s tail
* @list: the entry to move
* @head: the head that will follow our entry
*/
//将list移动到head前面
static inline void list_move_tail(struct list_head *list,
					struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
* list_empty – tests whether a list is empty
* @head: the list to test.
*/
//空链表判断
static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

//将list链表与head链表进行拼接
//拼接完的链表头是head
//比如：list: 1 2 3 4 5 6 7
//      head: 11 22 33 44
//      结果:head : 1 2 3 4 5 6 7 11 22 33 44
static inline void __list_splice(struct list_head *list,
					struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
* list_splice – join two lists
* @list: the new list to add.
* @head: the place to add it in the first list.
*/
static inline void list_splice(struct list_head *list, struct list_head *head)
{
if (!list_empty(list))
__list_splice(list, head);
}

/**
* list_splice_init – join two lists and reinitialise the emptied list.
* @list: the new list to add.
* @head: the place to add it in the first list.
*
* The list at @list is reinitialised
*/
static inline void list_splice_init(struct list_head *list,
struct list_head *head)
{
if (!list_empty(list)) {
__list_splice(list, head);
INIT_LIST_HEAD(list);
}
}

/**
* list_entry – get the struct for this entry
* @ptr:    the &struct list_head pointer.
* @type:    the type of the struct this is embedded in.
* @member:    the name of the list_struct within the struct.
*/
//小结构体指针ptr，大结构体类型type，小结构体在大结构体内部的成员名
#define list_entry(ptr, type, member) \
((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
* list_for_each    -    iterate over a list
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/

//遍历链表的for循环，每一次循环体内得到的就是一个小结构体指针
#define list_for_each(pos, head) \
for (pos = (head)->next; pos != (head); \
pos = pos->next)

/**
* list_for_each_prev    -    iterate over a list backwards
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/
//从后往前遍历
#define list_for_each_prev(pos, head) \
for (pos = (head)->prev; pos != (head); \
pos = pos->prev)

/**
* list_for_each_safe    -    iterate over a list safe against removal of list entry
* @pos:    the &struct list_head to use as a loop counter.
* @n:        another &struct list_head to use as temporary storage
* @head:    the head for your list.
*/
//安全的遍历，防止在循环体中有对节点的删除操作，这样会导致下一次进入循环体出现段错误
#define list_for_each_safe(pos, n, head) \
for (pos = (head)->next, n = pos->next; pos != (head); \
pos = n, n = pos->next)

/**
* list_for_each_entry    -    iterate over list of given type
* @pos:    the type * to use as a loop counter.
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/
//遍历过程中，得到大结构体指针:
//pos是遍历过程中的大结构体指针变量，
//head是小结构体链表头指针
//member小结构体在大结构体里面的成员名
#define list_for_each_entry(pos, head, member)                \
for (pos = list_entry((head)->next, typeof(*pos), member);    \
&pos->member != (head);                     \
pos = list_entry(pos->member.next, typeof(*pos), member))

/**
* list_for_each_entry_safe – iterate over list of given type safe against removal of list entry
* @pos:    the type * to use as a loop counter.
* @n:        another type * to use as temporary storage
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/
#define list_for_each_entry_safe(pos, n, head, member)            \
for (pos = list_entry((head)->next, typeof(*pos), member),    \
n = list_entry(pos->member.next, typeof(*pos), member);    \
&pos->member != (head);                     \
pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif
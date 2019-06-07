#ifndef __RTE_LINKLIST_H
#define __RTE_LINKLIST_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_LL == 1
	#include <stdint.h>
	/*Dummy type to make handling easier*/
	typedef uint8_t ll_node_t;
	/*Description of a linked list*/
	typedef struct
	{
		uint32_t n_size;
		ll_node_t* head;
		ll_node_t* tail;
	}linklist_t;
	/**********************
	 * GLOBAL PROTOTYPES
	 **********************/
	/**
	 * Initialize linked list
	 * @param ll_dsc pointer to ll_dsc variable
	 * @param node_size the size of 1 node in bytes
	 */
	void LinkList_Init(linklist_t * ll_p, uint32_t node_size);

	/**
	 * Add a new head to a linked list
	 * @param ll_p pointer to linked list
	 * @return pointer to the new head
	 */
	void * LinkList_InsHead(linklist_t * ll_p);

	/**
	 * Insert a new node in front of the n_act node
	 * @param ll_p pointer to linked list
	 * @param n_act pointer a node
	 * @return pointer to the new head
	 */
	void * LinkList_InsPrev(linklist_t * ll_p, void * n_act);

	/**
	 * Add a new tail to a linked list
	 * @param ll_p pointer to linked list
	 * @return pointer to the new tail
	 */
	void * LinkList_InsTail(linklist_t * ll_p);

	/**
	 * Remove the node 'node_p' from 'll_p' linked list.
	 * It does not free the the memory of node.
	 * @param ll_p pointer to the linked list of 'node_p'
	 * @param node_p pointer to node in 'll_p' linked list
	 */
	void LinkList_Remove(linklist_t  * ll_p, void * node_p);

	/**
	 * Remove and free all elements from a linked list. The list remain valid but become empty.
	 * @param ll_p pointer to linked list
	 */
	void LinkList_Clear(linklist_t * ll_p);

	/**
	 * Move a node to a new linked list
	 * @param ll_ori_p pointer to the original (old) linked list
	 * @param ll_new_p pointer to the new linked list
	 * @param node pointer to a node
	 */
	void LinkList_MoveList(linklist_t * ll_ori_p, linklist_t * ll_new_p, void * node);

	/**
	 * Return with head node of the linked list
	 * @param ll_p pointer to linked list
	 * @return pointer to the head of 'll_p'
	 */
	void * LinkList_GetHead(const linklist_t * ll_p);

	/**
	 * Return with tail node of the linked list
	 * @param ll_p pointer to linked list
	 * @return pointer to the head of 'll_p'
	 */
	void * LinkList_GetTail(const linklist_t * ll_p);

	/**
	 * Return with the pointer of the next node after 'n_act'
	 * @param ll_p pointer to linked list
	 * @param n_act pointer a node
	 * @return pointer to the next node
	 */
	void * LinkList_GetNext(const linklist_t * ll_p, const void * n_act);

	/**
	 * Return with the pointer of the previous node after 'n_act'
	 * @param ll_p pointer to linked list
	 * @param n_act pointer a node
	 * @return pointer to the previous node
	 */
	void * LinkList_GetPrev(const linklist_t * ll_p, const void * n_act);

	/**
	 * Move a nodw before an other node in the same linked list
	 * @param ll_p pointer to a linked list
	 * @param n_act pointer to node to move
	 * @param n_after pointer to a node which should be after `n_act`
	 */
	void LinkList_Movebefore(linklist_t * ll_p, void * n_act, void * n_after);

	/**********************
	 *      MACROS
	 **********************/

	#define LinkList_Read(list, i) for(i = LinkList_GetHead(&list); i != NULL; i = LinkList_GetNext(&list, i))

	#define LinkList_Readback(list, i) for(i = LinkList_GetTail(&list); i != NULL; i = LinkList_GetPrev(&list, i))
	#endif
#ifdef __cplusplus
}
#endif
#endif

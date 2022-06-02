#ifndef __KERNEL_LIST_H__
#define __KERNEL_LIST_H__
#include <stdlib.h>
#include <string.h>
// #include <stddef.h>
#include "list.h"
#define GENDER_BUF_SIZE 10

typedef struct PersonalInfo
{
    int age;
    char gender[GENDER_BUF_SIZE];
    float beauty; 
}DataType;

//双向循环链表节点
typedef struct Node 	//大结构体，结构体里面存放数据跟小结构体
{
	DataType data;			//数据域
	struct list_head list;	//小结构体，存放的是大结构体的前驱指针和后继指针
}LinkNode;

LinkNode* Get_AdminNode_By_Name__(const char *name);
int Delete_AdminNode_By_Num__(int num);
int Is_AdminUser_Name_Exist__(const char *name);
int Is_LinkNode_Empty(LinkNode *head);

int InsertNodeTail(LinkNode *head,LinkNode *new);
LinkNode * CreateNode(DataType data);
LinkNode * InitLinkNode();
void KernelNodeClear();

#endif
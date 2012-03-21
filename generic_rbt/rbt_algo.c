/* Static generic implementation for the core routines of Red Black
 * Tree by Xinhao.Yuan(xinhaoyuan at gmail dot com). Copyright of the
 * code is dedicated to the Public Domain */

#include <stdio.h>
#include <stdlib.h>

#define NODE_COUNT 50

int _count;
typedef struct node_s *node_t;
node_t _root;
struct node_s {
	 int data;
	 int rank;
	 node_t parent, left, right;
} _node[NODE_COUNT];

#define __RBT_NodeType   node_t
#define __RBT_DataType    int
#define __RBT_NodeNull   NULL
#define __RBT_TouchNode
#define __RBT_NewRedNode(k) ({ node_t node = _node + (_count ++); node->data = k; node->parent = node->left = node->right = __RBT_NodeNull; node->rank = -1; node; })
#define __RBT_SwapNodeContent(a,b) ({int t = (a)->data; (a)->data = (b)->data; (b)->data = t;})
#define __RBT_GetRank(n)      ((n) == __RBT_NodeNull ? 1 : (n)->rank)
#define __RBT_SetRank(n,r)    (n)->rank = (r)
#define __RBT_CompareData(k,n) (k == (n)->data ? 0 : (k < (n)->data ? -1 : 1))
#define __RBT_AcquireParentAndDir(n,p,d)					\
	 {														\
		  (p) = (n)->parent;								\
		  (d) = (p) == __RBT_NodeNull ? DIR_ROOT :			\
			   ((p)->left == (n) ? DIR_LEFT : DIR_RIGHT);	\
	 }
#define __RBT_GetLeftChild(n)    ((n)->left)
#define __RBT_GetRightChild(n)   ((n)->right)
#define __RBT_SetLeftChild(n,c)  ({ (n)->left = (c); if ((c) != __RBT_NodeNull) (c)->parent = (n); })
#define __RBT_SetRightChild(n,c) ({ (n)->right = (c); if ((c) != __RBT_NodeNull) (c)->parent = (n); })
#define __RBT_SetChildren(n,l,r) ({ (n)->left = (l); if ((l) != __RBT_NodeNull) (l)->parent = (n); (n)->right = (r); if ((r) != __RBT_NodeNull) (r)->parent = (n); })
#define __RBT_SetLeftChildFromRightChild(n,p)							\
	 { (n)->left = (p)->right; if ((n)->left) (n)->left->parent = (n); }
#define __RBT_SetRightChildFromLeftChild(n,p)							\
	 { (n)->right = (p)->left; if ((n)->right) (n)->right->parent = (n); }
#define __RBT_ThrowException(msg)					\
	 { printf("Exception: %s\n", msg); exit(-1); }

#include "rbt_algo.h"


void
print(__RBT_NodeType node, int last_rank)
{
	 int rank = __RBT_GetRank(node);
	 if (last_rank > 0)
	 {
		  if (rank != last_rank - 1 &&
			  rank != -last_rank + 1)
			   printf("\nERROR\n");
	 }
	 else if (last_rank < 0)
	 {
		  if (rank != -last_rank)
			   printf("\nERROR\n");
	 }
	 if (node == __RBT_NodeNull) return;
		 
	 fputc('(', stdout);
	 print(__RBT_GetLeftChild(node), node->rank);
	 printf("[%d,%d]", node->rank, node->data);
	 print(__RBT_GetRightChild(node), node->rank);
	 fputc(')', stdout);
	 if (node->parent == __RBT_NodeNull) printf("\n");
}

int perm[NODE_COUNT];

int
main(void)
{
	 _count = 0;
	 _root  = __RBT_NodeNull;

	 int i;
	 __RBT_NodeType node;

	 for (i = 0; i != NODE_COUNT; ++ i)
		  perm[i] = i;
	 for (i = 0; i != NODE_COUNT; ++ i)
	 {
		  int pos = (((unsigned int)rand() << 16) + rand()) % NODE_COUNT;
		  int t = perm[0]; perm[0] = perm[pos]; perm[pos] = t;
	 }
	 
	 for (i = 0; i != NODE_COUNT; ++ i)
	 {
		  _root = __RBT_Insert(_root, &node, perm[i]);
		  _root->parent = __RBT_NodeNull;
	 }

	 for (i = NODE_COUNT * 0.2; i != NODE_COUNT * 0.8; ++ i)
	 {
		  _root = __RBT_Remove(_root, &node, perm[i]);
		  _root->parent = __RBT_NodeNull;
	 }

	 /* for (i = NODE_COUNT * 0.2; i != NODE_COUNT * 0.8; ++ i) */
	 /* { */
	 /* 	  _root = __RBT_Insert(_root, &node, perm[i]); */
	 /* 	  _root->parent = __RBT_NodeNull; */
	 /* } */

	 print(_root, -__RBT_GetRank(_root));
	 	 
	 return 0;
}

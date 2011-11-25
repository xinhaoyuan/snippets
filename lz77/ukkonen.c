/* Time-stamp: <2011-02-12 14:07:48 xinhaoyuan> */
/* ukkonen.c: Implementation of Ukkonen's suffix tree construction */
#include "ukkonen.h"
#include <stdlib.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define HASH_MOD 3999997
#define HASH(ptr, ch) (((unsigned int)ptr * 3000001 + ch * 1000001) % HASH_MOD)

void
ukkonen_context_init(struct ukkonen_context_t *cont)
{
	 cont->root.suffix_link.node = &cont->root;
	 cont->root.suffix_link.ab_length = 0;

	 cont->free_ori_head = (struct uk_suffix_node_t *)
		  malloc(sizeof(struct uk_suffix_node_t));
	 cont->free_ori_head->free_next = NULL;

	 cont->free_hash_ori_head = (struct uk_hash_node_t *)
		  malloc(sizeof(struct uk_hash_node_t));
	 cont->free_hash_ori_head->free_next = NULL;

	 cont->hash_heads = (struct uk_hash_node_t **)
		  malloc(sizeof(struct uk_hash_node_t *) * HASH_MOD);
}

void
ukkonen_context_reset(struct ukkonen_context_t *cont, unsigned char *data)
{
	 cont->cur.node = &cont->root;
	 cont->cur.ab_length = 0;
	
	 cont->data = data;
	 cont->ab_pos = 0;
	 cont->free_head = cont->free_ori_head;
	 cont->free_hash_head = cont->free_hash_ori_head;
	 
	 int i;
	 for (i = 0; i != HASH_MOD; ++ i)
	 {
		  cont->hash_heads[i] = NULL;
	 }

	 cont->suffix_link_fix_count = 0;
}

inline unsigned char
ukkonen_get_ab(unsigned char *data, unsigned int pos)
{
	 unsigned char result = data[pos >> AB_PER_CHAR_SHIFT];
	 return (result >> ((pos & (AB_PER_CHAR - 1)) << AB_BITWIDTH_SHIFT)) & AB_BITMASK;
}

inline struct uk_suffix_node_t *
ukkonen_alloc_node(struct ukkonen_context_t *cont)
{
	 struct uk_suffix_node_t *node = cont->free_head;
	 if (node->free_next == NULL)
	 {
		 node->free_next = (struct uk_suffix_node_t *)malloc(sizeof(struct uk_suffix_node_t));
		 node->free_next->free_next = NULL;
	 }
	 cont->free_head = node->free_next;
	 node->suffix_link.node = NULL;
	 return node;
}

inline struct uk_hash_node_t *
ukkonen_hash_node_find(struct ukkonen_context_t *cont, struct uk_suffix_node_t *node, unsigned char ab, int touch)
{
	 unsigned int hash = HASH(node, ab);
	 struct uk_hash_node_t *cur = cont->hash_heads[hash];

	 while (cur != NULL)
	 {
		  if (cur->node == node && cur->ab == ab) break;
		  cur = cur->next;
	 }

	 if (cur == NULL)
	 {
		  
		  if (!touch)
		  {
			   return NULL;
		  }

		  cur = cont->free_hash_head;
		  if (cur->free_next == NULL)
		  {
			   cur->free_next = (struct uk_hash_node_t *)malloc(sizeof(struct uk_hash_node_t));
			   cur->free_next->free_next = NULL;
		  }
		  cont->free_hash_head = cur->free_next;

		  cur->node = node;
		  cur->ab = ab;

		  cur->next = cont->hash_heads[hash];
		  cont->hash_heads[hash] = cur;
	 }

	 return cur;
}

/* static inline void */
/* ukkonen_suffix_link_fix(struct ukkonen_context_t *cont, struct uk_suffix_position_t *link); */

static inline int
ukkonen_toward_ab(struct ukkonen_context_t *cont,
				  struct uk_suffix_position_t *cur,
				  unsigned char *cur_ab,
				  unsigned char *edge_ab)
{
	 *cur_ab = ukkonen_get_ab(cont->data, cont->ab_pos);
			   
	 if (cur->ab_length == 0)
	 {
		  struct uk_hash_node_t *edge_hash = ukkonen_hash_node_find(cont, cur->node, *cur_ab, 0);
		  if (edge_hash == NULL)
			   return -1;
		  else
		  {
			   cur->edge = &edge_hash->edge;
			   if (++ cur->ab_length == cur->edge->ab_length)
			   {
					cur->node = cur->edge->node;
					cur->ab_length = 0;
			   }
		  }
	 }
	 else
	 {
		  *edge_ab = ukkonen_get_ab(cont->data, cur->edge->ab_pos + cur->ab_length);
		  if (*edge_ab == *cur_ab)
		  {
			   if (++ cur->ab_length == cur->edge->ab_length)
			   {
					cur->node = cur->edge->node;
					cur->ab_length = 0;
			   }
		  }
		  else return -1;
	 }

	 ++ cont->ab_pos;
	 return 0;
}

static inline void
ukkonen_suffix_link_fix(struct ukkonen_context_t *cont, struct uk_suffix_node_t *node)
{
	 struct uk_suffix_position_t cur = node->suffix_link;
	 
	 while (cur.ab_length > 0 && cur.edge->ab_length > 0 &&
	 		cur.edge->ab_length <= cur.ab_length)
	 {
	 	  ++ cont->suffix_link_fix_count;
	 	  cur.ab_length -= cur.edge->ab_length;
	 	  cur.node = cur.edge->node;
	 	  if (cur.ab_length > 0)
	 	  {
	 		   struct uk_hash_node_t *hash_node =
	 				ukkonen_hash_node_find(cont, cur.node,
	 									   ukkonen_get_ab(cont->data, cur.edge->ab_pos + cur.edge->ab_length), 0);
	 		   if (hash_node == NULL)
	 		   {
	 				/* FATAL HERE */
	 		   }
	 		   cur.edge = &hash_node->edge;
	 	  }
	 }

	 while (1)
	 {
		  if (node == &cont->root) break;
		  if (node->suffix_link.ab_length == 0 ||
			  node->suffix_link.edge->ab_length == 0 ||
			  node->suffix_link.edge->ab_length > node->suffix_link.ab_length)
			   break;
	 
		  node->suffix_link = cur;

		  if (node->parent_node == &cont->root) break;
		  
		  unsigned int back_ab_len = node->parent_edge->ab_length;
		  while (back_ab_len > 0)
		  {
			   if (back_ab_len <= cur.ab_length)
			   {
					cur.ab_length -= back_ab_len;
					break;
			   }
			   else
			   {
					back_ab_len -= cur.ab_length;
					
					cur.edge = cur.node->parent_edge;
					cur.node = cur.node->parent_node;
					cur.ab_length = cur.edge->ab_length;
			   }
		  }
		  
		  node = node->parent_node;
	 }	 
}

void
ukkonen_toward_char(struct ukkonen_context_t *cont)
{
	 struct uk_suffix_position_t cur = cont->cur;
	 int root_visited = 0;
	 unsigned int i = 0;
	 while (1)
	 {
		  unsigned char cur_ab, edge_ab;
		  for ( ; i < AB_PER_CHAR; ++ i)
			   if (ukkonen_toward_ab(cont, &cur, &cur_ab, &edge_ab))
					break;
		  if (i == AB_PER_CHAR) break;

		  if (cur.ab_length == 0)
		  {
			   struct uk_hash_node_t *edge_hash =
					ukkonen_hash_node_find(cont, cur.node, cur_ab, 1);
			   
			   edge_hash->edge.ab_pos    = cont->ab_pos;
			   edge_hash->edge.ab_length = 0;
			   edge_hash->edge.node      = &cont->root;

			   root_visited = cur.node == &cont->root;

			   ukkonen_suffix_link_fix(cont, cur.node);
		  }
		  else
		  {
			   struct uk_suffix_node_t *node = ukkonen_alloc_node(cont);
			   struct uk_hash_node_t *edge_hash =
					ukkonen_hash_node_find(cont, node, edge_ab, 1);
			   edge_hash->edge.ab_pos        = cur.edge->ab_pos + cur.ab_length;
			   edge_hash->edge.ab_length     = cur.edge->ab_length == 0 ? 0 :
					cur.edge->ab_length - cur.ab_length;
			   edge_hash->edge.node          = cur.edge->node;

			   if (cur.edge->node != &cont->root)
			   {
					cur.edge->node->parent_edge = &edge_hash->edge;
					cur.edge->node->parent_node = node;
			   }

			   edge_hash = ukkonen_hash_node_find(cont, node, cur_ab, 1);

			   edge_hash->edge.ab_pos    = cont->ab_pos;
			   edge_hash->edge.ab_length = 0;
			   /* root indicate the leaf node */
			   edge_hash->edge.node      = &cont->root;

			   cur.edge->ab_length = cur.ab_length;
			   cur.edge->node      = node;

			   node->parent_edge = cur.edge;
			   node->parent_node = cur.node;

			   /* Update the suffix link */
			   ukkonen_suffix_link_fix(cont, cur.node);
			   node->suffix_link = cur.node->suffix_link;
			   
			   unsigned int ab_len = cur.ab_length;
			   unsigned int ab_pos = cur.edge->ab_pos;

			   if (cur.node == &cont->root)
			   {
					ab_len -= AB_PER_CHAR;
					ab_pos += AB_PER_CHAR;
			   }
			   
			   while (ab_len > 0)
			   {
					if (node->suffix_link.ab_length == 0)
					{
						 struct uk_hash_node_t *edge_hash =
							  ukkonen_hash_node_find(
								   cont,
								   node->suffix_link.node,
								   ukkonen_get_ab(cont->data, ab_pos),
								   0);
						 if (edge_hash == NULL)
						 {
							  /* FATAL HERE */
						 }
						 node->suffix_link.edge = &(edge_hash->edge);
					}

					unsigned int e_ab_len =
						 node->suffix_link.edge->ab_length;
					if (e_ab_len == 0)
					{
						 node->suffix_link.ab_length += ab_len;
						 ab_len = 0;
					}
					else if (e_ab_len - node->suffix_link.ab_length > ab_len)
					{
						 node->suffix_link.ab_length += ab_len;
						 ab_len = 0;
					}
					else
					{
						 ab_len -= e_ab_len - node->suffix_link.ab_length;
						 ab_pos += e_ab_len - node->suffix_link.ab_length;
						 
						 node->suffix_link.ab_length = 0;
						 node->suffix_link.node = 
							  node->suffix_link.edge->node;
					}
			   }

			   cur.node = node;
			   cur.ab_length = 0;
		  }

		  cur = cur.node->suffix_link;
		  if (cur.node == &cont->root && cur.ab_length == 0)
		  {
			   if (root_visited)
			   {
					cont->ab_pos += AB_PER_CHAR - i;
					break;
			   }
			   else root_visited = 1;
		  }
	 }
	 cont->cur = cur;
}

void
ukkonen_match(struct ukkonen_context_t *cont, unsigned char *data, unsigned int ab_length,
			  unsigned int *r_ab_pos, unsigned int *r_ab_length)
{
	 struct uk_suffix_node_t *m_node = &cont->root;
	 struct uk_suffix_edge_t *m_edge = NULL;
	 unsigned int m_ab_pos = 0;
	 unsigned int m_ab_length = 0;
	 unsigned int ab_pos;
	 for (ab_pos = 0; ab_pos != ab_length; ++ ab_pos)
	 {
		  unsigned char cur_ab = ukkonen_get_ab(data, ab_pos);
		  
		  if (m_ab_length == 0)
		  {
			   struct uk_hash_node_t *edge_hash = ukkonen_hash_node_find(cont, m_node, cur_ab, 0);
			   if (edge_hash == NULL)
					break;
			   else
			   {
					m_edge = &edge_hash->edge;
					m_ab_pos = m_edge->ab_pos - ab_pos;
					if (++ m_ab_length == m_edge->ab_length)
					{
						 m_node = m_edge->node;
						 m_ab_length = 0;
					}
			   }
		  }
		  else if (m_edge->ab_pos + m_ab_length < cont->ab_pos)
		  {
			   unsigned char edge_ab = ukkonen_get_ab(cont->data, m_edge->ab_pos + m_ab_length);
			   if (edge_ab == cur_ab)
			   {
					if (++ m_ab_length == m_edge->ab_length)
					{
						 m_node = m_edge->node;
						 m_ab_length = 0;
					}
			   }
			   else break;
		  }
		  else break;
	 }
	 *r_ab_pos    = m_ab_pos;
	 *r_ab_length = ab_pos;
}

#if UKKONEN_DEBUG

#include <stdio.h>

#define DUMP_TREE 0

struct char_stack_t
{
	 char buf[10240];
	 int  top;
};

static void
ukkonen_node_dump(struct ukkonen_context_t *cont, struct char_stack_t *s,
				  struct uk_suffix_node_t *node)
{
	 int top = s->top;
	 unsigned int i;
	 if (top > 0 && node == &cont->root)
	 {
#if !DUMP_TREE
		  s->buf[s->top] = 0;
		  printf("%s\n", &s->buf[0]);
#endif
	 }
	 else
	 {
#if DUMP_TREE
		  printf("%08x(", node);
#endif

		  for (i = 0; i != (1 << AB_BITWIDTH); ++ i)
		  {
			   struct uk_hash_node_t *edge_hash = ukkonen_hash_node_find(cont, node, i, 0);
			   if (edge_hash == NULL) continue;
			   unsigned int length = edge_hash->edge.ab_length;
			   if (length == 0) length = cont->ab_pos - edge_hash->edge.ab_pos;
#if DUMP_TREE
			   fputs(" [", stdout);
#endif
			   unsigned int j;
			   for (j = 0; j != length; ++ j)
			   {
#if DUMP_TREE
					fputc(ukkonen_get_ab(cont->data, edge_hash->edge.ab_pos + j), stdout);
#endif
					s->buf[s->top ++] = ukkonen_get_ab(cont->data, edge_hash->edge.ab_pos + j);
			   }
#if DUMP_TREE
			   fputc(']', stdout);
#endif
			   ukkonen_node_dump(cont, s, edge_hash->edge.node);
			   s->top = top;
		  }
#if DUMP_TREE
		  fputs(") ", stdout);
#endif
	 }
}

void
ukkonen_context_dump(struct ukkonen_context_t *cont)
{
	 struct char_stack_t s;
	 s.top = 0;
	 ukkonen_node_dump(cont, &s, &cont->root);
#if DUMP_TREE
	 printf("\n");
#endif
}

#endif

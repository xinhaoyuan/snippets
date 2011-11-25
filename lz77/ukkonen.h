#ifndef _UKKONEN_
#define _UKKONEN_

/* #define CHAR_BITWIDTH      8 */
/* #define AB_BITMASK         0xf */
/* #define AB_BITWIDTH        4 */
/* #define AB_BITWIDTH_SHIFT  2 */
/* #define AB_PER_CHAR        2 */
/* #define AB_PER_CHAR_SHIFT  1 */

#define CHAR_BITWIDTH      8
#define AB_BITMASK         0xff
#define AB_BITWIDTH        8
#define AB_BITWIDTH_SHIFT  3
#define AB_PER_CHAR        1
#define AB_PER_CHAR_SHIFT  0

struct uk_suffix_position_t
{
	 struct uk_suffix_node_t *node;
	 struct uk_suffix_edge_t *edge;
	 unsigned int  ab_length;
};

struct uk_suffix_edge_t
{
	 unsigned int ab_pos;
	 unsigned int ab_length;
	 struct uk_suffix_node_t *node;
};

struct uk_suffix_node_t
{
	 struct uk_suffix_position_t suffix_link;
	 struct uk_suffix_node_t *parent_node;
	 struct uk_suffix_edge_t *parent_edge;
	 struct uk_suffix_node_t *free_next;
};

struct uk_hash_node_t
{
	 struct uk_suffix_node_t *node;
	 unsigned char ab;

	 struct uk_suffix_edge_t edge;

	 struct uk_hash_node_t *next;
	 struct uk_hash_node_t *free_next;
};

struct ukkonen_context_t
{
	 unsigned char *data;
	 unsigned int  ab_pos;
	 struct uk_suffix_position_t cur;
	 struct uk_suffix_node_t root;
	 struct uk_hash_node_t **hash_heads;

	 struct uk_suffix_node_t *free_head, *free_ori_head;
	 struct uk_hash_node_t *free_hash_head, *free_hash_ori_head;

	 unsigned int suffix_link_fix_count;
};

void ukkonen_context_init(struct ukkonen_context_t *cont);
void ukkonen_context_reset(struct ukkonen_context_t *cont, unsigned char *data);
void ukkonen_toward_char(struct ukkonen_context_t *cont);
#if UKKONEN_DEBUG
void ukkonen_context_dump(struct ukkonen_context_t *cont);
#endif

void ukkonen_match(struct ukkonen_context_t *cont, unsigned char *data, unsigned int ab_length,
				   unsigned int *r_ab_pos, unsigned int *r_ab_length);

#endif

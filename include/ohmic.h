/* ohmic: a fairly reliable hashmap library
 * Copyright (c) 2013 Cyphar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __OHMIC_H__
#define __OHMIC_H__

/* hashmap structures */
struct ohm_node {
	void *key;
	size_t keylen;

	void *value;
	size_t valuelen;

	struct ohm_node *next;
};

struct ohm_t {
	struct ohm_node **table;
	int count;
	int size;
	int (*hash)(void *, size_t);
};

struct ohm_iter {
	void *key;
	size_t keylen;

	void *value;
	size_t valuelen;

	struct ohm_iter_internal {
		struct ohm_t *hashmap;
		struct ohm_node *node;
		int index;
	} internal;
};

/* basic hashmap functionality */
struct ohm_t *ohm_init(int, int (*)(void *, size_t));
void ohm_free(struct ohm_t *);

void *ohm_search(struct ohm_t *, void *, size_t);

void *ohm_insert(struct ohm_t *, void *, size_t, void *, size_t);
int ohm_remove(struct ohm_t *, void *, size_t);

struct ohm_t *ohm_resize(struct ohm_t *, int);

/* functions to iterate (correctly) through the hashmap */
struct ohm_iter ohm_iter_init(struct ohm_t *);
void ohm_iter_inc(struct ohm_iter *);

/* functions to copy, duplicate and merge hashmaps */
struct ohm_t *ohm_dup(struct ohm_t *);
void ohm_cpy(struct ohm_t *, struct ohm_t *);

void ohm_merge(struct ohm_t *, struct ohm_t *);

/* default hashing function (modulo of djb2 hash -- not reccomended) */
int ohm_hash(void *, size_t);

#endif /* __OHMIC_H__ */

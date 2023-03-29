/**
 * The author of this software is David R. Hanson.
 *
 * Copyright (c) 1994,1995,1996,1997 by David R. Hanson. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose, subject to the provisions described below, without fee is
 * hereby granted, provided that this entire notice is included in all
 * copies of any software that is or includes a copy or modification of
 * this software and in all copies of the supporting documentation for
 * such software.

 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY REPRESENTATION OR
 * WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS SOFTWARE OR
 * ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * David Hanson / drh@microsoft.com / http://www.research.microsoft.com/~drh/
 * $Id: CPYRIGHT,v 1.2 1997/11/04 22:31:40 drh Exp $
 *
 * Modified at 2009 by Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 */

#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"

#include "set.h"

#define INT_MAX	0x0fffffff

#define T Set_o

static unsigned int hashatom(const void *x)
{
    return *((unsigned int *) x);
}

int Set_sizeof_obj()
{
    return sizeof (struct Set_o) + sizeof (struct member);
}

int allocated = 0;

/** Set Constructor
 *
 * Creates a new set. If hint is bigger than 0, a hash table is created
 * using the hashfield function as the function to extract the parameter we want
 * to use as hash index. If hint is 0, hashfield parameters is ignored.
 */
T
Set_new(int hint, void * hashfield(const void *x))
{
    T set;
    int i;
    static int primes[] ={47, 71, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, INT_MAX};
    assert(hint >= 0);
    if (hint) {
        assert(hashfield);

        for (i = 1; primes[i] < hint; i++)
            ;

        set = hwapi_mem_alloc(sizeof (*set) + primes[i - 1] * sizeof (set->buckets[0]));
        if (!set) {
            return NULL;
        }

        set->size = primes[i - 1];
        set->hashfield = hashfield;
        set->hash = hashatom; /* could take this as a parameter */
    } else {
        set = hwapi_mem_alloc(set_sizeof);
        if (!set) {
            return NULL;
        }
        set->size = 1;
        set->buckets = (struct member **) (set + 1);
        set->hashfield = NULL;
        set->hash = NULL;
    }
    for (i = 0; i < set->size; i++) {
        set->buckets[i] = NULL;
    }

    set->length = 0;
    set->timestamp = 0;
    return set;
}

/** Check if is member
 *
 * This function returns true if the object pointed by member 
 * is in the set. The comparation is made by the address, not 
 * by its contents, use the function Set_find() to search a member
 * by comparation.
 */

int Set_member(T set, const void *member)
{
    int i;
    struct member **pp;

    assert(set);
    assert(member);

    if (set->hash) {
        /* as now member referes to a full member, not a field */
        i = (set->hash)(set->hashfield(member)) % set->size;
    } else {
        i = 0;
    }

    for (pp = &set->buckets[i]; *pp; pp = &(*pp)->link) {
        if (*pp == member) {
            return 1;
        }
    }

    return 0;
}

/** Get a member
 * 
 * Returns member "i" of an 1-dimensional (no hash) set
 *
 */
void * Set_get(T set, int i)
{
    int j = 0;
    struct member *p;
    assert(set && i >= 0);

    for (p = set->buckets[0]; j < i; j++) {
        if (p) {
            p = p->link;
        }

    }
    if (p) {
        return (void *) p->member;
    } else {
        return NULL;
    }
}

/** Finds a member.
 * 
 * Looks for a member in the set comparing the contents.
 * The contents are extracted from the saved members using the field
 * function and compared using the cmp function
 */
void * Set_find(T set, const void *data, int cmp(const void *x, const void *y))
{
    int i;
    struct member *p;
    assert(set && data && cmp);

    if (set->hash) {
        /* here member pointer should be the same field where the hash was taken from hashfield function
         */
        i = (set->hash)(data) % set->size;
    } else {
        i = 0;
    }
    for (p = set->buckets[i]; p; p = p->link) {
        if (cmp(data, p->member) == 0) {
            break;
        }
    }
    if (p) {
        return (void *) p->member;
    } else {
        return NULL;
    }
}

/** Finds a member using 2 comparations.
 * 
 * Like the Set_find() function but realises 2 comparations at the same time.
 */
void * Set_find2(T set, const void *data1, const void *data2, int cmp1(const void *x, const void *y), int cmp2(const void *x, const void *y))
{
    int i;
    struct member *p;
    assert(set && data1 && cmp1 && cmp2);

    if (set->hash) {
        /* here member pointer should be the same field where the hash was taken from hashfield function
         */
        /* watch out here!! we just compare 1 or also 2?? */
        i = (set->hash)(data1) % set->size;
    } else {
        i = 0;
    }

    for (p = set->buckets[i]; p; p = p->link) {
        if (cmp1(data1, p->member) == 0 && cmp2(data2, p->member) == 0) {
            break;
        }
    }
    if (p) {
        return (void *) p->member;
    } else {
        return NULL;
    }
}

/** Put a member
 *
 * Adds the new member to the set. Note, there is any copy here
 * just the pointer is added. 
 * 
 * Hash will be taken using the hashfield function given at the constructor
 */
void Set_put(T set, void *member)
{
    int i;
    struct member *p;
    assert(set);
    assert(member);

    if (set->hash) {
        /* here hash field is taken using the function on creation */
        i = (set->hash)(set->hashfield(member)) % set->size;
    } else {
        i = 0;
    }

    NEW(p);
    if (p) {
        p->member = member;
        p->link = set->buckets[i];
        set->buckets[i] = p;
        set->length++;
        set->timestamp++;
    }
}

/** Remove a member
 *
 * Given a pointer to a member in the set (the address), it 
 * is removed from the set, but not the element itself.
 *
 * @return !=NULL pointer to the element if found and deleted
 * @return =NULL if element not found 
 */
void * Set_remove(T set, const void *member)
{
    int i;
    struct member *p, *q;
    assert(set);
    assert(member);

    set->timestamp++;

    if (set->hash) {
        /* as now member referes  */
        i = (set->hash)(set->hashfield(member)) % set->size;
    } else {
        i = 0;
    }

    q = set->buckets[i];

    for (p = set->buckets[i]; p; p = p->link) {
        if (p->member == member) {
            if (q != p) {
                q->link = p->link;
            } else {
                set->buckets[i] = p->link;
            }

            DELETE(p);

            set->length--;

            return (void *) member;
        }

        q = p;
    }
    xprintf("SET: CAUTION Anything removed.\n");
    return NULL;
}

/** Get Set length 
 */
int Set_length(T set)
{
    assert(set);
    return set->length;
}

void Set_clear(T set)
{
    assert(set);
    if ((set)->length > 0) {
        int i;
        struct member *p, *q;
        for (i = 0; i < (set)->size; i++) {
            for (p = (set)->buckets[i]; p; p = q) {
                q = p->link;
                DELETE(p);
            }
            set->buckets[i] = NULL;
        }
        set->length = 0;
    }
}

/** Free Set 
 */
void Set_delete(T * set)
{
    assert(set && *set);
    Set_clear(*set);
    DELETE(*set);
    set = NULL;
}

/** Destroy Set 
 * Frees Set and all its members using the callback function elem_delete()
 */
extern void Set_destroy(T * set, void elem_delete(void **x))
{
    int i;
    struct member *p, *q;
    assert(set && *set);
    for (i = 0; i < (*set)->size; i++) {
        for (p = (*set)->buckets[i]; p; p = q) {
            q = p->link;
            elem_delete((void **) & p->member);
            DELETE(p);
        }
    }
    DELETE(*set);
    set = NULL;
}

T Set_dup(T set, void * elem_dup(const void *x))
{
    T newset;
    int i, j;
    struct member *q;
    assert(set && elem_dup);

    newset = Set_new(set->hashfield ? set->size : 0, set->hashfield);
    if (!newset) {
        return NULL;
    }

    for (i = 0; i < set->size; i++) {
        for (q = set->buckets[i]; q; q = q->link) {
            struct member *p;
            const void *member = q->member;
            if (set->hash) {
                j = (set->hash) (set->hashfield(member)) % set->size;
            } else {
                j = 0;
            }
            NEW(p);
            if (!p) {
                Set_delete(&newset);
                /** @todo does not have access to object's delete method
                 * dupped objects remain in memory
                 */
                return NULL;
            } else {
                p->member = elem_dup(member);
                p->link = newset->buckets[j];
                newset->buckets[j] = p;
                newset->length++;
            }
        }
    }
    return newset;
}

/** Copy Set to packet buffer 
 *
 * Copies all members usinge the elem_topkt() function applied to
 * each of them into a linear buffer, returns total number of bytes
 * copied.
 * \warning Only 1-dimensional sets (size=0) are possible (no hash)
 *
 * @return amount of bytes copied, -1 if error
 */
int Set_topkt(void *x, char **start, char *end, int elem_topkt(void *x, char **start, char *end))
{
    T set = (T) x;
    struct member *q;

    assert(set && elem_topkt && start && end);
    assert(end >= *start);

    if (end == *start) {
        return 0;
    }

    if (set->length > 127) {
        xprintf("Error, sets with more than 127 elements can't be saved in a packet\n");
        return 0;
    }

    /** @todo length IS INTEGER!!!
     */
    **start = set->length;
    (*start)++;

    if (!set->length)
        return 1;

    for (q = set->buckets[0]; q; q = q->link) {
        if (!elem_topkt(q->member, start, end)) {
            return 0;
        }
    }

    return 1;
}

extern void * Set_newfrompkt(char **start, char *end, void * elem_newfrompkt(char **start, char *end))
{

    int i;
    T set;
    void *elem;
    char len;
    assert(start && end && elem_newfrompkt);
    assert(end >= *start);


    if (end == *start) {
        return NULL;
    }

    set = Set_new(0, NULL);
    if (!set) {
        return NULL;
    }

    len = **start;
    (*start)++;

    for (i = 0; i < len; i++) {
        elem = elem_newfrompkt(start, end);
        if (elem) {
            Set_put(set, elem);
        } else {
            Set_delete(&set);
            return NULL;
        }
    }
    return (void *) set;
}

/** Get total size of Set (sum of all elements size)
 */
int Set_sizeof(T set, int elem_sizeof(const void *x))
{
    int totalsz = 0;
    assert(set);
    if ((set)->length > 0) {
        int i;
        struct member *p, *q;
        for (i = 0; i < set->size; i++) {
            for (p = set->buckets[i]; p; p = q) {
                q = p->link;
                totalsz += elem_sizeof((void *) p->member);
            }
        }
    }
    return totalsz + 1; /* because first work indicates set length */
}

void ** Set_toArray(T set, void *end)
{
    int i, j = 0;
    void **array;
    struct member *p;
    assert(set);
    array = calloc(1, (set->length + 1) * sizeof (*array));
    for (i = 0; i < set->size; i++) {
        for (p = set->buckets[i]; p; p = p->link) {
            array[j++] = (void *) p->member;
        }
    }
    array[j] = end;
    return array;
}

#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "uthash.h"
#include "utlist.h"
#include "kvconstants.h"
#include "kvcacheset.h"
#include "kvstore.h"

/* Initializes CACHESET to hold a maximum of ELEM_PER_SET elements.
 * ELEM_PER_SET must be at least 2.
 * Returns 0 if successful, else a negative error code. */
int kvcacheset_init(kvcacheset_t *cacheset, unsigned int elem_per_set) {
  int ret;
  if (elem_per_set < 2) {
    return -1;
  }

  cacheset->elem_per_set = elem_per_set;
  if ((ret = pthread_rwlock_init(&cacheset->lock, NULL)) < 0) {
    return ret;
  }
  
  cacheset->num_entries = 0;
  cacheset->entries = NULL;
  cacheset->hash = NULL;
  return 0;
}


/* Get the entry corresponding to KEY from CACHESET. Returns 0 if successful,
 * else returns a negative error code. If successful, populates VALUE with a
 * malloced string which should later be freed. */
int kvcacheset_get(kvcacheset_t *cacheset, char *key, char **value) {
  struct kvcacheentry *elt;
  HASH_FIND_STR(cacheset->hash, key, elt);

  if (!elt) {
    return ERRNOKEY;
  }

  elt->refbit = true;
  *value = malloc(strlen(elt->value)+1);
  strcpy(*value, elt->value);
  return 0;
}

/* Add the given KEY, VALUE pair to CACHESET. Returns 0 if successful, else
 * returns a negative error code. Should evict elements if necessary to not
 * exceed CACHESET->elem_per_set total entries. */
int kvcacheset_put(kvcacheset_t *cacheset, char *key, char *value) {
  struct kvcacheentry *selected;
  struct kvcacheentry *elt, *tmp;

  /* Check if an entry with the key already exists. If it does, set the refbit
   * and replace its value, otherwise allocate a new entry. */
  HASH_FIND_STR(cacheset->hash, key, selected);
  if (selected) {
    free(selected->value);
    selected->refbit = true;
  } else {
    selected = malloc(sizeof(struct kvcacheentry));
    selected->key = malloc(strlen(key)+1);
    if (!selected->key) {
      return ERRFILCRT;
    }

    strcpy(selected->key, key);
    selected->refbit = false;
  }


  /* Copy the value into the entry field. */
  selected->value = malloc(strlen(value)+1);
  if (!selected->value) {
    return ERRFILCRT;
  }
  strcpy(selected->value, value);


  /* If adding a new entry, check for evictions. */
  if (!selected->refbit) {
    if (cacheset->num_entries < cacheset->elem_per_set) {
      cacheset->num_entries++;
    } else {
      DL_FOREACH_SAFE(cacheset->entries, elt, tmp) {
        DL_DELETE(cacheset->entries, elt);

        if (!elt->refbit) {
          HASH_DEL(cacheset->hash, elt);
          free(elt->key);
          free(elt->value);
          free(elt);
          break;
        }
          
        elt->refbit = false;
        DL_APPEND(cacheset->entries, elt);
      }
    }

    DL_APPEND(cacheset->entries, selected);
    HASH_ADD_KEYPTR(hh, cacheset->hash, selected->key, 
      strlen(selected->key), selected);
  }
  return 0;
}

/* Deletes the entry corresponding to KEY from CACHESET. Returns 0 if
 * successful, else returns a negative error code. */
int kvcacheset_del(kvcacheset_t *cacheset, char *key) {
  struct kvcacheentry *elt;

  HASH_FIND_STR(cacheset->hash, key, elt);
  if (elt) {
    HASH_DEL(cacheset->hash, elt);
    DL_DELETE(cacheset->entries, elt);
    free(elt);
    return 0;
  }

  return ERRNOKEY;
}

/* Completely clears this cache set. For testing purposes. */
void kvcacheset_clear(kvcacheset_t *cacheset) {
  struct kvcacheentry *elt, *tmp;

  HASH_ITER(hh, cacheset->hash, elt, tmp) {
    HASH_DEL(cacheset->hash, elt);
    DL_DELETE(cacheset->entries, elt);
    free(elt);
  }
}

/* Returns refbit of key. For testing purposes. */
int kvcacheset_refbit(kvcacheset_t *cacheset, char *key) {
  struct kvcacheentry *entry;
  HASH_FIND_STR(cacheset->hash, key, entry);

  if (!entry)
    return -1;
  return entry->refbit;
}
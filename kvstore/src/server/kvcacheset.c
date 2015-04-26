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

  if (!elt)
    return ERRNOKEY;

  *value = malloc(strlen(elt->value)+1);
  strcpy(*value, elt->value);
  return 0;
}

/* Add the given KEY, VALUE pair to CACHESET. Returns 0 if successful, else
 * returns a negative error code. Should evict elements if necessary to not
 * exceed CACHESET->elem_per_set total entries. */
int kvcacheset_put(kvcacheset_t *cacheset, char *key, char *value) {
  struct kvcacheentry *new_entry;
  struct kvcacheentry *elt;

  /* Replace an old entry if overwritten. */
  HASH_FIND_STR(cacheset->hash, key, elt);
  if (elt) {
    elt->value = value;
    return 0;
  }

  new_entry = malloc(sizeof(struct kvcacheentry));
  if (!new_entry) {
    return ERRFILLEN;
  }

  /* Initialize values in the entry. */
  new_entry->key = key;
  new_entry->value = value;

  /* Check for evictions. */
  if (cacheset->num_entries < cacheset->elem_per_set) {
    cacheset->num_entries++;
  } else {
    DL_FOREACH(cacheset->entries, elt) {
      DL_DELETE(cacheset->entries, elt);

      if (!elt->refbit)
        HASH_DEL(cacheset->hash, elt);
        free(elt);
        break;

      elt->refbit = false;
      DL_APPEND(cacheset->entries, elt);
    }
  }

  DL_APPEND(cacheset->entries, new_entry);
  HASH_ADD_KEYPTR(hh, cacheset->hash, key, strlen(key), new_entry);
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






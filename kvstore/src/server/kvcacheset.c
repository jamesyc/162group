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
  return 0;
}


/* Get the entry corresponding to KEY from CACHESET. Returns 0 if successful,
 * else returns a negative error code. If successful, populates VALUE with a
 * malloced string which should later be freed. */
int kvcacheset_get(kvcacheset_t *cacheset, char *key, char **value) {
  struct kvcacheentry *elt;

  DL_FOREACH(cacheset->entries, elt) {
    if (!strcmp(elt->key, key)) {
      *value = malloc(strlen(elt->value)+1);
      strcpy(*value, elt->value);
      return 0;
    }
  }

  return ERRNOKEY;
}

/* Add the given KEY, VALUE pair to CACHESET. Returns 0 if successful, else
 * returns a negative error code. Should evict elements if necessary to not
 * exceed CACHESET->elem_per_set total entries. */
int kvcacheset_put(kvcacheset_t *cacheset, char *key, char *value) {
  struct kvcacheentry *new_entry;
  struct kvcacheentry *el, *tmp;

  /* Replace an old entry if overwritten. */
  DL_FOREACH_SAFE(cacheset->entries, el, tmp) {
    if (!strcmp(el->key, key)) {
      el->value = value;
      return 0;
    }
  }

  new_entry = malloc(sizeof(struct kvcacheentry));

  if (new_entry == NULL) {
    return ERRFILLEN;
  }

  /* Initialize values in the entry. */
  new_entry->key = key;
  new_entry->value = value;

  /* Check for evictions. */
  if (cacheset->num_entries < cacheset->elem_per_set) {
    cacheset->num_entries++;
  } else {
    DL_FOREACH(cacheset->entries, el) {
      DL_DELETE(cacheset->entries, el);

      if (!el->refbit)
        break;

      el->refbit = false;
      DL_APPEND(cacheset->entries, el);
    }
  }

  DL_APPEND(cacheset->entries, new_entry);
  return 0;
}

/* Deletes the entry corresponding to KEY from CACHESET. Returns 0 if
 * successful, else returns a negative error code. */
int kvcacheset_del(kvcacheset_t *cacheset, char *key) {
  struct kvcacheentry *elt;

  DL_FOREACH(cacheset->entries, elt) {
    if (!strcmp(elt->key, key)) {
      DL_DELETE(cacheset->entries, elt);
      return 0;
    }
  }

  return ERRNOKEY;
}

/* Completely clears this cache set. For testing purposes. */
void kvcacheset_clear(kvcacheset_t *cacheset) {
  struct kvcacheentry *elt, *tmp;

  DL_FOREACH_SAFE(cacheset->entries, elt, tmp) {
    DL_DELETE(cacheset->entries, elt);
  }
}






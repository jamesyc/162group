#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "kvconstants.h"
#include "kvcache.h"
#include "kvstore.h"
#include "kvmessage.h"
#include "kvserver.h"
#include "tpclog.h"
#include "socket_server.h"

/* Initializes a kvserver. Will return 0 if successful, or a negative error
 * code if not. DIRNAME is the directory which should be used to store entries
 * for this server.  The server's cache will have NUM_SETS cache sets, each
 * with ELEM_PER_SET elements.  HOSTNAME and PORT indicate where SERVER will be
 * made available for requests.  USE_TPC indicates whether this server should
 * use TPC logic (for PUTs and DELs) or not. */
int kvserver_init(kvserver_t *server, char *dirname, unsigned int num_sets,
    unsigned int elem_per_set, unsigned int max_threads, const char *hostname,
    int port, bool use_tpc) {
  int ret;
  ret = kvcache_init(&server->cache, num_sets, elem_per_set);
  if (ret < 0) return ret;
  ret = kvstore_init(&server->store, dirname);
  if (ret < 0) return ret;
  if (use_tpc) {
      ret = tpclog_init(&server->log, dirname);
      if (ret < 0) return ret;
  }
  server->hostname = malloc(strlen(hostname) + 1);
  if (server->hostname == NULL)
    return ENOMEM;
  strcpy(server->hostname, hostname);
  server->port = port;
  server->use_tpc = use_tpc;
  server->max_threads = max_threads;
  server->handle = kvserver_handle;
  return 0;
}

/* Sends a message to register SERVER with a TPCMaster over a socket located at
 * SOCKFD which has previously been connected. Does not close the socket when
 * done. Returns -1 if an error was encountered.
 *
 * Checkpoint 2 only. */
int kvserver_register_master(kvserver_t *server, int sockfd) {
  kvmessage_t *respmsg, regmsg;

  /* Make register message */
  memset(&regmsg, 0, sizeof(kvmessage_t));

  regmsg.type = REGISTER;

  regmsg.key = calloc(1, strlen(server->hostname) + 1);
  strcpy(regmsg.key, server->hostname);

  char portstr[15];
  sprintf(portstr, "%d", server->port);
  regmsg.value = calloc(1, strlen(portstr) + 1);
  strcpy(regmsg.value, portstr);

  /* Send and cleanup register message */
  kvmessage_send(&regmsg, sockfd);
  if (regmsg.key != NULL)
    free(regmsg.key);
  if (regmsg.value != NULL)
    free(regmsg.value);

  /* Recieve and parse master response message */
  respmsg = kvmessage_parse(sockfd);
  int error = 0;
  if (respmsg != NULL) {
    if (strcmp(respmsg->message, MSG_SUCCESS) != 0) {
      error = -1;
    }
    kvmessage_free(respmsg);
  }

  return error;
}

/* Attempts to get KEY from SERVER. Returns 0 if successful, else a negative
 * error code.  If successful, VALUE will point to a string which should later
 * be free()d.  If the KEY is in cache, take the value from there. Otherwise,
 * go to the store and update the value in the cache. */
int kvserver_get(kvserver_t *server, char *key, char **value) {
  int success;
  pthread_rwlock_t *lock;

  if (strlen(key) > MAX_KEYLEN)
    return ERRKEYLEN;

  lock = kvcache_getlock(&server->cache, key);

  pthread_rwlock_rdlock(lock);
  success = kvcache_get(&server->cache, key, value);
  pthread_rwlock_unlock(lock);
  
  /* If the key is not in the cache, go to the store. */
  if (success < 0) {
    success = kvstore_get(&server->store, key, value);
    
    if (success == 0) {
      pthread_rwlock_wrlock(lock);
      kvcache_put(&server->cache, key, *value);
      pthread_rwlock_unlock(lock);
    }
  }

  return success;
}

/* Checks if the given KEY, VALUE pair can be inserted into this server's
 * store. Returns 0 if it can, else a negative error code. */
int kvserver_put_check(kvserver_t *server, char *key, char *value) {
  return kvstore_put_check(&server->store, key, value);
}

/* Inserts the given KEY, VALUE pair into this server's store and cache. Access
 * to the cache should be concurrent if the keys are in different cache sets.
 * Returns 0 if successful, else a negative error code. */
int kvserver_put(kvserver_t *server, char *key, char *value) {
  int err;
  pthread_rwlock_t *lock;

  err = kvserver_put_check(server, key, value);
  if (err < 0)
    return err;

  err = kvstore_put(&server->store, key, value);
  if (err < 0)
    return err;

  lock = kvcache_getlock(&server->cache, key);

  pthread_rwlock_wrlock(lock);
  err = kvcache_put(&server->cache, key, value);
  pthread_rwlock_unlock(lock);

  return err;
}

/* Checks if the given KEY can be deleted from this server's store.
 * Returns 0 if it can, else a negative error code. */
int kvserver_del_check(kvserver_t *server, char *key) {
  return kvstore_del_check(&server->store, key);
}

/* Removes the given KEY from this server's store and cache. Access to the
 * cache should be concurrent if the keys are in different cache sets. Returns
 * 0 if successful, else a negative error code. */
int kvserver_del(kvserver_t *server, char *key) {
  int success;
  pthread_rwlock_t *lock;

  lock = kvcache_getlock(&server->cache, key);

  pthread_rwlock_wrlock(lock);
  success = kvcache_del(&server->cache, key);
  pthread_rwlock_unlock(lock);

  success = kvstore_del(&server->store, key);
  return success;
}

/* Returns an info string about SERVER including its hostname and port. */
char *kvserver_get_info_message(kvserver_t *server) {
  char info[1024], buf[256];
  time_t ltime = time(NULL);
  strcpy(info, asctime(localtime(&ltime)));
  sprintf(buf, "{%s, %d}", server->hostname, server->port);
  strcat(info, buf);
  char *msg = malloc(strlen(info));
  strcpy(msg, info);
  return msg;
}

/* Handles an incoming kvmessage REQMSG, and populates the appropriate fields
 * of RESPMSG as a response. RESPMSG and REQMSG both must point to valid
 * kvmessage_t structs. Assumes that the request should be handled as a TPC
 * message. This should also log enough information in the server's TPC log to
 * be able to recreate the current state of the server upon recovering from
 * failure.  See the spec for details on logic and error messages.
 *
 * Checkpoint 2 only. */
void kvserver_handle_tpc(kvserver_t *server, kvmessage_t *reqmsg,
    kvmessage_t *respmsg) {
  respmsg->type = RESP;
  respmsg->message = ERRMSG_NOT_IMPLEMENTED;
}

/* Handles an incoming kvmessage REQMSG, and populates the appropriate fields
 * of RESPMSG as a response. RESPMSG and REQMSG both must point to valid
 * kvmessage_t structs. Assumes that the request should be handled as a non-TPC
 * message. See the spec for details on logic and error messages. */
void kvserver_handle_no_tpc(kvserver_t *server, kvmessage_t *reqmsg,
    kvmessage_t *respmsg) {

  int error;
  char **value = malloc(sizeof(char **));

  /* Set default response type. */
  respmsg->type = RESP;

  if (reqmsg->type == GETREQ) {
    error = kvserver_get(server, reqmsg->key, value);
    if (!error) {
      respmsg->type = GETRESP;
      respmsg->key = reqmsg->key;
      respmsg->value = *value;
    }
  } else if (reqmsg->type == PUTREQ) {
    error = kvserver_put(server, reqmsg->key, reqmsg->value);
  } else if (reqmsg->type == DELREQ) {
    error = kvserver_del(server, reqmsg->key);
  }

  if (!error) {
    respmsg->message = MSG_SUCCESS;
  } else {
    respmsg->message = GETMSG(error);
  }
  
}

/* Generic entrypoint for this SERVER. Takes in a socket on SOCKFD, which
 * should already be connected to an incoming request. Processes the request
 * and sends back a response message.  This should call out to the appropriate
 * internal handler. */
void kvserver_handle(kvserver_t *server, int sockfd, void *extra) {
  kvmessage_t *reqmsg, *respmsg;
  respmsg = calloc(1, sizeof(kvmessage_t));
  reqmsg = kvmessage_parse(sockfd);
  void (*server_handler)(kvserver_t *server, kvmessage_t *reqmsg,
      kvmessage_t *respmsg);
  server_handler = server->use_tpc ?
    kvserver_handle_tpc : kvserver_handle_no_tpc;
  if (reqmsg == NULL) {
    respmsg->type = RESP;
    respmsg->message = ERRMSG_INVALID_REQUEST;
  } else {
    server_handler(server, reqmsg, respmsg);
  }
  kvmessage_send(respmsg, sockfd);
  if (reqmsg != NULL)
    kvmessage_free(reqmsg);
}

/* Restore SERVER back to the state it should be in, according to the
 * associated LOG.  Must be called on an initialized  SERVER. Only restores the
 * state of the most recent TPC transaction, assuming that all previous actions
 * have been written to persistent storage. Should restore SERVER to its exact
 * state; e.g. if SERVER had written into its log that it received a PUTREQ but
 * no corresponding COMMIT/ABORT, after calling this function SERVER should
 * again be waiting for a COMMIT/ABORT.  This should also ensure that as soon
 * as a server logs a COMMIT, even if it crashes immediately after (before the
 * KVStore has a chance to write to disk), the COMMIT will be finished upon
 * rebuild. The cache need not be the same as before rebuilding.
 *
 * Checkpoint 2 only. */
int kvserver_rebuild_state(kvserver_t *server) {
  return -1;
}

/* Deletes all current entries in SERVER's store and removes the store
 * directory.  Also cleans the associated log. */
int kvserver_clean(kvserver_t *server) {
  return kvstore_clean(&server->store);
}

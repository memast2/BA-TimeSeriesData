#ifndef _DEBUG_H
#define _DEBUG_H

/*
 * Thanks to Zed Shaw for:
 * http://c.learncodethehardway.org/book/ex20.html
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

#define xstr(s) str(s)
#define str(s) #s

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%s:%d): " M "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define IF_DEBUG(CODE)
#else
#define IF_DEBUG(CODE) CODE
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%s:%d: errno: %s) " M "\n", __FILE__,  __func__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%s:%d: errno: %s) " M "\n", __FILE__,  __func__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%s:%d) " M "\n", __FILE__, __func__,  __LINE__, ##__VA_ARGS__)

#define fail(M, ...) do { log_err(M, ##__VA_ARGS__); errno=0; goto error; } while (0)

#define check(A, M, ...) if (!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_not_null(A) if ((A) == NULL) { log_err(xstr(A) " cannot be null"); errno=0; goto error; }

#define check_not_null_msg(A, M, ...) if ((A) == NULL) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  do { log_err(M, ##__VA_ARGS__); errno=0; goto error; } while (0)

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if (!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_debug_not_null(A) if ((A) == NULL) { debug(xstr(A) " cannot be null"); errno=0; goto error; }

#define debug_ts_sim(alg,ts1,ts2)           \
  debug("%s([%3d,%2d], [%3d,%2d])", alg,    \
      ts1->station_id, ts1->value_type_id,  \
      ts2->station_id, ts2->value_type_id)

#endif /* _DEBUG_H */

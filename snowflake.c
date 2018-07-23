/*
  +----------------------------------------------------------------------+
  | Snowflake                                                            |
  +----------------------------------------------------------------------+
  | Copyright (c) 2016 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: hook <hikdo7@gmail.com>                                      |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <stdint.h>
#include <sched.h>

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"

int pid  = -1;
int ncpu = 1;
int shmid = -1;

atomic_t *lock;
shmdat_t *shmdat;

ZEND_DECLARE_MODULE_GLOBALS(snowflake)

/* {{{ DL support */
#ifdef COMPILE_DL_SNOWFLAKE
ZEND_GET_MODULE(snowflake)
#endif
/* }}} */

/* {{{ snowflake_functions[] */
zend_function_entry snowflake_functions[] = {
    PHP_FE(snowflake_nextid, NULL)
    PHP_FE(snowflake_desc,   NULL)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ snowflake_module_entry */
zend_module_entry snowflake_module_entry = {
    STANDARD_MODULE_HEADER,
    "snowflake",
    snowflake_functions,
    PHP_MINIT(snowflake),
    PHP_MSHUTDOWN(snowflake),
    PHP_RINIT(snowflake),
    NULL,
    PHP_MINFO(snowflake),
    PHP_SNOWFLAKE_VERSION,
    PHP_MODULE_GLOBALS(snowflake),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/* {{{ PHP_INI */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("snowflake.node",  "0", PHP_INI_SYSTEM, OnUpdateLong,  node, zend_snowflake_globals, snowflake_globals)
PHP_INI_END()
/* }}} */


/* {{{ php_snowflake_init_globals */
static void php_snowflake_init_globals(zend_snowflake_globals *snowflake_globals)
{
    snowflake_globals->initialized = 0;
    snowflake_globals->epoch       = 1420864633000ULL;
}
/* }}} */


static uint64_t get_time_in_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec) * 1000ULL + ((uint64_t)tv.tv_usec) / 1000ULL;
}

static uint64_t till_next_ms(uint64_t last_ts)
{
    uint64_t ts;
    while((ts = get_time_in_ms()) <= last_ts);
    return ts;
}

static uint64_t key2int(char *key)
{
    uint64_t v;
    if (sscanf(key, "%llu", &v) == 0) {
        return 0;
    }
    return v;
}

void shmtx_lock(atomic_t *lock, int pid)
{
    int i, n;
    for ( ;; ) {
        if (*lock == 0 && __sync_bool_compare_and_swap(lock, 0, pid)) {
            return;
        }
        if (ncpu > 1) {
            for (n = 1; n < 2048; n <<= 1) {
                for (i = 0; i < n; i++) {
                    __asm("pause");
                }    
                if (*lock == 0 && __sync_bool_compare_and_swap(lock, 0, pid)) {
                    return;
                }
            }
        }
        sched_yield();
    }
}


void shmtx_unlock(atomic_t *lock, int pid)
{
    __sync_bool_compare_and_swap(lock, pid, 0);
}


int snowflake_init()
{		
    key_t key = ftok("/sbin/init", 0x07);
    shmid = shmget(key, sizeof(atomic_t) + sizeof(shmdat_t), IPC_CREAT | IPC_EXCL | 0600);	
    if (shmid == -1) {
        if (errno == EEXIST && (shmid = shmget(key, 0, 0)) != -1) {
            lock = (atomic_t *) shmat(shmid, NULL, 0);
            shmdat = (shmdat_t *) (lock + sizeof(atomic_t));
            if (strcmp(shmdat->name, "snowflake") == 0) {
                return SUCCESS;
            }
        }
        php_error_docref(NULL, E_WARNING, "create shared memory segment failed '%s'", strerror(errno));
        return FAILURE;		
    }		
    lock   = (atomic_t *) shmat(shmid, NULL, 0);
    shmdat = (shmdat_t *) (lock + sizeof(atomic_t));	
    *lock = 0;
    shmdat->sequence  = 0;
    shmdat->timestamp = 0;
    strcpy(shmdat->name, "snowflake");	
    return SUCCESS;
}

void snowflake_shutdown()
{
    if (*(lock) == pid) {
        shmtx_unlock(lock, pid);
    }
    shmdt((void *)lock);
    shmctl(shmid, IPC_RMID, NULL);
}


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(snowflake)
{
    ZEND_INIT_MODULE_GLOBALS(snowflake, php_snowflake_init_globals, NULL);	
    REGISTER_INI_ENTRIES();	
    if (SNOWFLAKE_G(node) < 0) {
        php_error_docref(NULL, E_WARNING, "snowflake.node must greater than 0");
        return FAILURE;
    }
    if (SNOWFLAKE_G(node) > 0x3FF) {
        php_error_docref(NULL, E_WARNING, "snowflake.node must less than %d", 0x3FF);
        return FAILURE;
    }	
    if (!SNOWFLAKE_G(initialized)) {
        if(snowflake_init() == FAILURE) {
            return FAILURE;
        }
    }
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpu <= 0) {
        ncpu = 1;
    }
    SNOWFLAKE_G(initialized) = 1;		
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(snowflake)
{
    if (pid == -1) {
        pid = (int)getpid();
    }
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(snowflake)
{
    if (SNOWFLAKE_G(initialized)) {
        snowflake_shutdown();
        SNOWFLAKE_G(initialized) = 0;
    }
    UNREGISTER_INI_ENTRIES();    
    return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(snowflake)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "snowflake support", "enabled");
    php_info_print_table_row(2, "Version", PHP_SNOWFLAKE_VERSION);
    php_info_print_table_row(2, "Supports", SNOWFLAKE_SUPPORT_EMAIL);
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ proto string snowflake_nextid() */
PHP_FUNCTION(snowflake_nextid)
{
    if (SNOWFLAKE_G(initialized) == 0) {
        RETURN_BOOL(0);
    }			
    shmtx_lock(lock, pid);
    int len;
    char *str = NULL;
    uint64_t ts = get_time_in_ms();		
    if (ts == shmdat->timestamp) {
        shmdat->sequence = (shmdat->sequence + 1) & 0xFFF;
        if(shmdat->sequence == 0) {
            ts = till_next_ms(ts);
        }
    } else {
        shmdat->sequence = 0;
    }	
    shmdat->timestamp = ts;	
    //php_error_docref(NULL, E_NOTICE, "pid:%d,ts:%llu,sequence:%d", pid, ts, shmdat->sequence);	
    uint64_t id = ((ts - SNOWFLAKE_G(epoch)) << 22) | ((SNOWFLAKE_G(node) & 0x3FF) << 12) | shmdat->sequence;	
    shmtx_unlock(lock, pid);			
    len = spprintf(&str, 0, "%llu", id);
    RETURN_STRINGL(str, len);
}
/* }}} */

/* {{{ proto array snowflake_desc(string key) */
PHP_FUNCTION(snowflake_desc)
{
    char *key;
    long ts;
    int len, node;
    uint64_t id;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &len TSRMLS_CC) == FAILURE) {
        RETURN_FALSE;
    }
    if (!(id = key2int(key))) {
        RETURN_FALSE;
    }
    node = (id >> 12) & 0x3FF;
    ts   = ((id >> 22) + SNOWFLAKE_G(epoch)) / 1000ULL;	
    array_init(return_value);
    add_assoc_long(return_value, "node", node);
    add_assoc_long(return_value, "timestamp", ts);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

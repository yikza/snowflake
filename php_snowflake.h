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
  | Author: hook <xhook7@gmail.com>                                      |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_SNOWFLAKE_H
#define PHP_SNOWFLAKE_H

#define PHP_SNOWFLAKE_VERSION "1.0"

extern zend_module_entry snowflake_module_entry;
#define phpext_snowflake_ptr &snowflake_module_entry

#ifdef PHP_WIN32
#define PHP_SNOWFLAKE_API __declspec(dllexport)
#else
#define PHP_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#define SNOWFLAKE_G(v) TSRMG(snowflake_globals_id, zend_snowflake_globals *, v)
#else
#define SNOWFLAKE_G(v) (snowflake_globals.v)
#endif

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
    int initialized;
	uint32_t node;
	uint64_t epoch;
ZEND_END_MODULE_GLOBALS(snowflake)

PHP_MINIT_FUNCTION(snowflake);
PHP_MSHUTDOWN_FUNCTION(snowflake);
PHP_RINIT_FUNCTION(snowflake);
PHP_RSHUTDOWN_FUNCTION(snowflake);
PHP_MINFO_FUNCTION(snowflake);

PHP_FUNCTION(snowflake_nextid);
PHP_FUNCTION(snowflake_desc);

typedef volatile unsigned int atomic_t;

typedef struct {
	uint32_t sequence;
	uint64_t timestamp;
	char name[10];
} shmdat_t;



#endif  /* PHP_SNOWFLAKE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

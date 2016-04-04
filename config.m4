dnl $Id$

PHP_ARG_ENABLE(snowflake, whether to enable snowflake support,
[  --enable-snowflake           Enable snowflake support])

if test "$PHP_SNOWFLAKE" != "no"; then
  PHP_NEW_EXTENSION(snowflake, snowflake.c, $ext_shared)
fi

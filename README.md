#snowflake

An implementation of twitter's snowflake in c , build in php extension.

### Install
```
$ /path/to/phpize
$ ./configure --with-php-config=/path/to/php-config
$ make && make install
```

### Ini
```
extension = snowflake.so
snowflake.node = 0
```

### Functions
```
1. snowflake_nextid()
2. snowflake_desc($id)
```

### Test
```
has tested on php7

```
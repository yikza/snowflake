--TEST--
Check for snowflake desc
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--INI--
snowflake.node=3
--FILE--
<?php 
$ts = time();
$id = snowflake_nextid();
$desc = snowflake_desc($id);
var_dump(is_array($desc));
var_dump($desc['node'] == 3);
var_dump($desc['timestamp'] == $ts);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
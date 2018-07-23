--TEST--
Check for snowflake
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--INI--
snowflake.node=3
--FILE--
<?php 
$ts = time();
$id = snowflake_nextid();
var_dump(is_numeric($id));
?>
--EXPECT--
bool(true)
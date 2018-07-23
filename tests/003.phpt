--TEST--
Check for snowflake
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--INI--
snowflake.node=0
--FILE--
<?php 
$ids = [];
for ($i=0; $i<1000; $i++) {
	$ids[] = snowflake_nextid();
}
$ids = array_unique($ids);
var_dump(count($ids) == 1000);
?>
--EXPECT--
bool(true)
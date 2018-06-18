--TEST--
Check for snowflake functions
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--INI--
snowflake.node=1
--FILE--
<?php 
$now = time();
$id = snowflake_nextid();
$desc = snowflake_desc($id);
var_dump($desc);
if (isset($desc['node'])) {
	var_dump($desc['node'] === 1);
}
if (isset($desc['timestamp'])) {
	var_dump($desc['timestamp'] == $now);
}
?>
--EXPECTF--
array(2) {
  ["node"]=>
  int(%d)
  ["timestamp"]=>
  float(%f)
}
bool(true)
bool(true)
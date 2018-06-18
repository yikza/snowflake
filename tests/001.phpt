--TEST--
Check for snowflake presence
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--INI--
snowflake.node=0
--FILE--
<?php 
echo "snowflake extension is available";
?>
--EXPECT--
snowflake extension is available
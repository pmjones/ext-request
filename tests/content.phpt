--TEST--
enable_post_data_reading: basic test
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--POST_RAW--
Content-Type: application/x-www-form-urlencoded
a=1&b=ZYX
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
var_dump($request->content);
--EXPECT--
string(9) "a=1&b=ZYX"
--TEST--
StdRequest - superglobals are copied
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new StdRequest();
$_SERVER['HTTP_HOST'] = 'NOT example.com';
var_dump($request->server['HTTP_HOST']);
--EXPECT--
string(11) "example.com"

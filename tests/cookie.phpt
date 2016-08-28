--TEST--
PhpRequest::$cookie
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--COOKIE--
foo=bar; baz=bat
--FILE--
<?php
$request = new PhpRequest();
var_dump($request->cookie['foo']);
var_dump($request->cookie['baz']);
--EXPECT--
string(3) "bar"
string(3) "bat"

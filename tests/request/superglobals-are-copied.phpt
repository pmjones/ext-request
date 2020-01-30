--TEST--
ServerRequest - superglobals are copied
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new ServerRequest($GLOBALS);
$_SERVER['HTTP_HOST'] = 'NOT example.com';
var_dump($request->server['HTTP_HOST']);
--EXPECT--
string(11) "example.com"

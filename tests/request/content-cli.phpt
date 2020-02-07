--TEST--
ServerRequest::$content (cli)
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new ServerRequest($GLOBALS);
var_dump($request->content);
$request = new ServerRequest($GLOBALS, 'foobar');
var_dump($request->content);
--EXPECT--
NULL
string(6) "foobar"

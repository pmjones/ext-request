--TEST--
ServerRequest::$cookie
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--COOKIE--
foo=bar; baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new ServerRequest();
var_dump($request->cookie['foo']);
var_dump($request->cookie['baz']);
--EXPECT--
string(3) "bar"
string(3) "bat"

--TEST--
ServerRequest::$xhr
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';

$request = new ServerRequest();
var_dump($request->xhr);

$_SERVER['HTTP_X_REQUESTED_WITH'] = 'XmlHttpRequest';
$request = new ServerRequest();
var_dump($request->xhr);
--EXPECT--
bool(false)
bool(true)

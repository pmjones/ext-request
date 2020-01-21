--TEST--
ServerRequest::$contentLength
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$request = new ServerRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => '123',
];
$request = new ServerRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => ' 123',
];
$request = new ServerRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => 'non-integer',
];
$request = new ServerRequest($GLOBALS);
var_dump($request->contentLength);

--EXPECT--
NULL
int(123)
int(123)
int(0)

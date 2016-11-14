--TEST--
ServerRequest::$contentType
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_CONTENT_TYPE' => 'text/plain',
];
$request = new ServerRequest();
var_dump($request->contentType, $request->contentCharset);

$_SERVER['HTTP_CONTENT_TYPE'] = 'text/plain;charset=utf-8';
$request = new ServerRequest();
var_dump($request->contentType, $request->contentCharset);

$_SERVER['HTTP_CONTENT_TYPE'] = 'text/plain;foo=bar';
$request = new ServerRequest();
var_dump($request->contentType, $request->contentCharset);

$_SERVER['HTTP_CONTENT_TYPE'] = 'text/plain;foo=bar;charset=utf-8;baz=dib';
$request = new ServerRequest();
var_dump($request->contentType, $request->contentCharset);
--EXPECT--
string(10) "text/plain"
NULL
string(10) "text/plain"
string(5) "utf-8"
string(10) "text/plain"
NULL
string(10) "text/plain"
string(5) "utf-8"


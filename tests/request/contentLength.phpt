--TEST--
ServerRequest::$contentLength
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
    'HTTP_CONTENT_LENGTH' => '123.456',
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
NULL
NULL

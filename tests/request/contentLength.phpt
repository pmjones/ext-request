--TEST--
SapiRequest::$contentLength
--FILE--
<?php
$request = new SapiRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => '123',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => '123.456',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->contentLength);

$_SERVER = [
    'HTTP_CONTENT_LENGTH' => 'non-integer',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->contentLength);

--EXPECT--
NULL
int(123)
NULL
NULL

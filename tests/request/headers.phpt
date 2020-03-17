--TEST--
SapiRequest::$headers
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_FOO_BAR_BAZ' => 'dib,zim,gir',
    'NON_HTTP_HEADER' => 'should not show',
    'CONTENT_LENGTH' => '123',
    'CONTENT_TYPE' => 'text/plain',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->headers);
--EXPECT--
array(4) {
  ["host"]=>
  string(11) "example.com"
  ["foo-bar-baz"]=>
  string(11) "dib,zim,gir"
  ["content-length"]=>
  string(3) "123"
  ["content-type"]=>
  string(10) "text/plain"
}

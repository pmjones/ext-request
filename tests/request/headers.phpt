--TEST--
StdRequest::$headers
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_FOO_BAR_BAZ' => 'dib,zim,gir',
    'NON_HTTP_HEADER' => 'should not show',
    'CONTENT_LENGTH' => '123',
    'CONTENT_TYPE' => 'text/plain',
];
$request = new StdRequest();
var_dump($request->headers);
--EXPECT--
array(4) {
  ["Host"]=>
  string(11) "example.com"
  ["Foo-Bar-Baz"]=>
  string(11) "dib,zim,gir"
  ["Content-Length"]=>
  string(3) "123"
  ["Content-Type"]=>
  string(10) "text/plain"
}

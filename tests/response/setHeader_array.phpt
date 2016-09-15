--TEST--
PhpResponse::setHeader (array)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new PhpResponse();
$response->setHeader('Cache-Control', [
    'public',
    'max-age' => '123',
    's-maxage' => '456',
    'no-cache',
]);
$response->setHeader('Content-Type', [
    'text/plain' => ['charset' => 'utf-8']
]);
var_dump($response->getHeaders());
--EXPECT--
array(2) {
  ["Cache-Control"]=>
  array(1) {
    [0]=>
    string(43) "public, max-age=123, s-maxage=456, no-cache"
  }
  ["Content-Type"]=>
  array(1) {
    [0]=>
    string(24) "text/plain;charset=utf-8"
  }
}
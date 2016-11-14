--TEST--
ServerResponse::setHeader (array)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$response = new ServerResponse();
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
  ["cache-control"]=>
  string(43) "public, max-age=123, s-maxage=456, no-cache"
  ["content-type"]=>
  string(24) "text/plain;charset=utf-8"
}

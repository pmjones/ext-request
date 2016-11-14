--TEST--
ServerResponse::addHeader
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
$response->addHeader('', 'should-not-show');
$response->addHeader('Should-Not-Show', '');
$response->addHeader('Foo-Bar', 'baz');
$response->addHeader('foo-bar', 'dib');
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["foo-bar"]=>
  string(8) "baz, dib"
}

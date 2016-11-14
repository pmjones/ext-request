--TEST--
ServerResponse::setHeader
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
PHPT_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$response = new ServerResponse();
$response->setHeader('', 'should-not-show');
$response->setHeader('Should-Not-Show', '');
$response->setHeader('Foo-Bar', 'baz');
$response->setHeader('foo-bar', 'dib');
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["foo-bar"]=>
  string(3) "dib"
}

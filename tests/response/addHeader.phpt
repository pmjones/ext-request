--TEST--
StdResponse::addHeader
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$response->addHeader('', 'should-not-show');
$response->addHeader('Should-Not-Show', '');
$response->addHeader('Foo-Bar', 'baz');
$response->addHeader('foo-bar', 'dib');
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["Foo-Bar"]=>
  array(2) {
    [0]=>
    string(3) "baz"
    [1]=>
    string(3) "dib"
  }
}

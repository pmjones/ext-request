--TEST--
ServerResponse::setHeader
--FILE--
<?php
$response = new ServerResponse();
$response->setHeader('Foo-Bar', 'baz');
$response->setHeader('foo-bar', 'dib');
var_dump($response->getHeaders());
$response->unsetHeaders();
var_dump($response->getHeaders());
$response->setHeader('Foo-Bar', 'baz');
$response->setHeader('dib-zim', 'gir');
var_dump($response->getHeaders());
$response->unsetHeader('foo-bar');
$response->unsetHeader('no-such');
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["foo-bar"]=>
  string(3) "dib"
}
NULL
array(2) {
  ["foo-bar"]=>
  string(3) "baz"
  ["dib-zim"]=>
  string(3) "gir"
}
array(1) {
  ["dib-zim"]=>
  string(3) "gir"
}

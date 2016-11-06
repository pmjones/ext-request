--TEST--
StdResponse::addHeader (array)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$response->addHeader('Foo', ['bar' => 'baz']);
$response->addHeader('Foo', ['dib' => ['zim', 'gir']]);
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["foo"]=>
  string(20) "bar=baz, dib;zim;gir"
}

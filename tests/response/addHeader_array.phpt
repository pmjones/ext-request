--TEST--
ServerResponse::addHeader (array)
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
$response->addHeader('Foo', ['bar' => 'baz']);
$response->addHeader('Foo', ['dib' => ['zim', 'gir']]);
var_dump($response->getHeaders());
--EXPECT--
array(1) {
  ["foo"]=>
  string(20) "bar=baz, dib;zim;gir"
}

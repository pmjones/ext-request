--TEST--
StdResponse::sendHeaders
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new StdResponse();
$response->setHeader('Foo', 'bar');
$response->setHeader('Baz', 'dib');
$response->addHeader('Baz', 'zim');
$response->send();
var_dump(headers_list());
// it appears EXPECTHEADERS can't handle duplicate headers
--EXPECTHEADERS--
foo: bar
baz: dib
baz: zim
--EXPECT--
array(3) {
  [0]=>
  string(8) "foo: bar"
  [1]=>
  string(8) "baz: dib"
  [2]=>
  string(8) "baz: zim"
}

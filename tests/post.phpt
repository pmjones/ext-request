--TEST--
PhpRequest::$post
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--POST--
foo=bar&baz=bat
--FILE--
<?php
$request = new PhpRequest();
var_dump($request->post);
--EXPECT--
array(2) {
  ["foo"]=>
  string(3) "bar"
  ["baz"]=>
  string(3) "bat"
}

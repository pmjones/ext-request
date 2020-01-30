--TEST--
ServerRequest::$post
--POST--
foo=bar&baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new ServerRequest($GLOBALS);
var_dump($request->post);
--EXPECT--
array(2) {
  ["foo"]=>
  string(3) "bar"
  ["baz"]=>
  string(3) "bat"
}

--TEST--
ServerRequest::$post
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--POST--
foo=bar&baz=bat
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';
$request = new ServerRequest();
var_dump($request->post);
--EXPECT--
array(2) {
  ["foo"]=>
  string(3) "bar"
  ["baz"]=>
  string(3) "bat"
}

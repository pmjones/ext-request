--TEST--
ServerRequest::$acceptCharset
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'HTTP_ACCEPT_CHARSET' => 'iso-8859-5;q=0.8, unicode-1-1',
];
$request = new ServerRequest();
var_dump($request->acceptCharset);
--EXPECTF--
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(11) "unicode-1-1"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(10) "iso-8859-5"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
}

--TEST--
ServerRequest::$accept
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
    'HTTP_ACCEPT' => 'application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1',
];
$request = new ServerRequest();
var_dump($request->accept);
--EXPECTF--
array(4) {
  [0]=>
  array(3) {
    ["value"]=>
    string(16) "application/json"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(1) {
      ["foo"]=>
      string(3) "bar"
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(15) "application/xml"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  array(3) {
    ["value"]=>
    string(6) "text/*"
    ["quality"]=>
    string(3) "0.2"
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  array(3) {
    ["value"]=>
    string(3) "*/*"
    ["quality"]=>
    string(3) "0.1"
    ["params"]=>
    array(0) {
    }
  }
}

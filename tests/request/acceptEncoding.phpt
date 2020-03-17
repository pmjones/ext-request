--TEST--
SapiRequest::$acceptEncoding
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'HTTP_ACCEPT_ENCODING' => 'compress;q=0.5, gzip;q=1.0',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->acceptEncoding);
--EXPECTF--
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(4) "gzip"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(8) "compress"
    ["quality"]=>
    string(3) "0.5"
    ["params"]=>
    array(0) {
    }
  }
}

--TEST--
ServerRequest::$forwarded (etc.)
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
    'HTTP_X_FORWARDED_FOR' => '1.2.3.4, 5.6.7.8, 9.10.11.12',
    'HTTP_X_FORWARDED_HOST' => 'example.net',
    'HTTP_X_FORWARDED_PROTO' => 'https',
    'HTTP_FORWARDED' => 'For="[2001:db8:cafe::17]:4711", for=192.0.2.60;proto=http;by=203.0.113.43, for=192.0.2.43, 12.34.56.78',
];

$request = new ServerRequest();
var_dump($request->forwardedFor);
var_dump($request->forwardedHost);
var_dump($request->forwardedProto);
var_dump($request->forwarded);
--EXPECT--
array(3) {
  [0]=>
  string(7) "1.2.3.4"
  [1]=>
  string(7) "5.6.7.8"
  [2]=>
  string(10) "9.10.11.12"
}
string(11) "example.net"
string(5) "https"
array(4) {
  [0]=>
  array(1) {
    ["for"]=>
    string(24) "[2001:db8:cafe::17]:4711"
  }
  [1]=>
  array(3) {
    ["for"]=>
    string(10) "192.0.2.60"
    ["proto"]=>
    string(4) "http"
    ["by"]=>
    string(12) "203.0.113.43"
  }
  [2]=>
  array(1) {
    ["for"]=>
    string(10) "192.0.2.43"
  }
  [3]=>
  array(0) {
  }
}

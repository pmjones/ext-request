--TEST--
SapiRequest::$url
--FILE--
<?php
// empty
$request = new SapiRequest([]);
var_dump($request->url);

$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'REQUEST_URI' => '/foo/bar?baz=dib',
    'SERVER_PORT' => '8080',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->url);

// without host
unset($_SERVER['HTTP_HOST']);
$request = new SapiRequest($GLOBALS);
var_dump($request->url);

// secure
$_SERVER = [
    'SERVER_NAME' => 'example.com',
    'HTTPS' => 'on',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->url['scheme']);

// hostn
$_SERVER = [
    'SERVER_NAME' => 'example.com',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->url['host']);

// host/port
$_SERVER = [
    'HTTP_HOST' => 'example.com:8080',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->url['host']);
var_dump($request->url['port']);

// server port
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'SERVER_PORT' => '8080',
];
$request = new SapiRequest($GLOBALS);
var_dump($request->url['host']);
var_dump($request->url['port']);

--EXPECTF--
array(0) {
}
array(8) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  string(11) "example.com"
  ["port"]=>
  int(8080)
  ["user"]=>
  NULL
  ["pass"]=>
  NULL
  ["path"]=>
  string(8) "/foo/bar"
  ["query"]=>
  string(7) "baz=dib"
  ["fragment"]=>
  NULL
}
array(8) {
  ["scheme"]=>
  string(4) "http"
  ["host"]=>
  NULL
  ["port"]=>
  int(8080)
  ["user"]=>
  NULL
  ["pass"]=>
  NULL
  ["path"]=>
  string(8) "/foo/bar"
  ["query"]=>
  string(7) "baz=dib"
  ["fragment"]=>
  NULL
}
string(5) "https"
string(11) "example.com"
string(11) "example.com"
int(8080)
string(11) "example.com"
int(8080)

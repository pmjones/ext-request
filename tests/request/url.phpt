--TEST--
ServerRequest::$url
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
    'REQUEST_URI' => '/foo/bar?baz=dib',
    'SERVER_PORT' => '8080',
];
$request = new ServerRequest();
var_dump($request->url);

// without host
unset($_SERVER['HTTP_HOST']);
try {
    $request = new ServerRequest();
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}

// secure
$_SERVER = [
    'SERVER_NAME' => 'example.com',
    'HTTPS' => 'on',
];
$request = new ServerRequest();
var_dump($request->url['scheme']);

// hostn
$_SERVER = [
    'SERVER_NAME' => 'example.com',
];
$request = new ServerRequest();
var_dump($request->url['host']);

// host/port
$_SERVER = [
    'HTTP_HOST' => 'example.com:8080',
];
$request = new ServerRequest();
var_dump($request->url['host']);
var_dump($request->url['port']);

// server port
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'SERVER_PORT' => '8080',
];
$request = new ServerRequest();
var_dump($request->url['host']);
var_dump($request->url['port']);

--EXPECTF--
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
string(16) "RuntimeException"
string(43) "Could not determine host for ServerRequest."
string(5) "https"
string(11) "example.com"
string(11) "example.com"
int(8080)
string(11) "example.com"
int(8080)

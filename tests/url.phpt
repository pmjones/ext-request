--TEST--
PhpRequest::$url
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER += [
    'HTTP_HOST' => 'example.com',
    'REQUEST_URI' => '/foo/bar?baz=dib',
    'SERVER_PORT' => '8080',
];
$request = new PhpRequest();
var_dump($request->url);

// without host
unset($_SERVER['HTTP_HOST']);
try {
    $request = new PhpRequest();
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}

// secure
$_SERVER = [
    'SERVER_NAME' => 'example.com',
    'HTTPS' => 'on',
];
$request = new PhpRequest();
var_dump($request->url->scheme);

// hostn
$_SERVER = [
    'SERVER_NAME' => 'example.com',
];
$request = new PhpRequest();
var_dump($request->url->host);

// host/port
$_SERVER = [
    'HTTP_HOST' => 'example.com:8080',
];
$request = new PhpRequest();
var_dump($request->url->host);
var_dump($request->url->port);

// server port
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'SERVER_PORT' => '8080',
];
$request = new PhpRequest();
var_dump($request->url->host);
var_dump($request->url->port);

--EXPECTF--
object(stdClass)#%d (8) {
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
string(40) "Could not determine host for PhpRequest."
string(5) "https"
string(11) "example.com"
string(11) "example.com"
int(8080)
string(11) "example.com"
int(8080)
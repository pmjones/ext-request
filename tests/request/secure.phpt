--TEST--
StdRequest::$secure
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'example.com';

// default
$request = new StdRequest();
var_dump($request->secure);

// https on
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTPS' => 'on',
];
$request = new StdRequest();
var_dump($request->secure);

// https off
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTPS' => 'off',
];
$request = new StdRequest();
var_dump($request->secure);

// port 443
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'SERVER_PORT' => '443',
];
$request = new StdRequest();
var_dump($request->secure);

// port 80
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'SERVER_PORT' => '80',
];
$request = new StdRequest();
var_dump($request->secure);

// forwarded https
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_X_FORWARDED_PROTO' => 'https',
];
$request = new StdRequest();
var_dump($request->secure);

// forwarded http
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'HTTP_X_FORWARDED_PROTO' => 'http',
];
$request = new StdRequest();
var_dump($request->secure);
--EXPECT--
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(false)

--TEST--
ServerRequest::__construct
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';

// Basic construct
$request = new ServerRequest();
var_dump(get_class($request));

// Globals argument
$fakeGlobals = array(
    '_ENV' => array(
        'c' => 'd',
    ),
    '_SERVER' => array(
        'HTTP_HOST' => 'foo.bar'
    ),
    '_GET' => array(
        'e' => 'f',
    ),
    '_POST' => array(
        'g' => 'h',
    ),
    '_FILES' => array(
        'i' => array(
            'tmp_name' => 'j'
        ),
    ),
    '_COOKIE' => array(
        'k' => 'l',
    ),
);
$request = new ServerRequest($fakeGlobals);
var_dump(
    $request->env === $fakeGlobals['_ENV'] &&
    $request->server === $fakeGlobals['_SERVER'] &&
    $request->get === $fakeGlobals['_GET'] &&
    $request->post === $fakeGlobals['_POST'] &&
    $request->files === $fakeGlobals['_FILES'] &&
    $request->cookie === $fakeGlobals['_COOKIE']
);

// Partial globals argument
$_SERVER['HTTP_HOST'] = 'foo.bar';
$request = new ServerRequest(array(
    '_GET' => array(
        'foo' => 'bar',
    )
));
var_dump($request->url['host']);
var_dump($request->get['foo']);

// Check for immutability in globals
$_GET['foo'] = new stdClass();
try {
    $request = new ServerRequest();
    echo 'fail';
} catch( UnexpectedValueException $e ) {}

// Check __construct can't be called twice
try {
    $request->__construct();
    echo 'fail';
} catch( RuntimeException $e ) {}

--EXPECT--
string(13) "ServerRequest"
bool(true)
string(7) "foo.bar"
string(3) "bar"

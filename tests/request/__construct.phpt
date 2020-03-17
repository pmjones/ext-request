--TEST--
SapiRequest::__construct
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';

// Basic construct
$request = new SapiRequest($GLOBALS);
var_dump(get_class($request));

// Globals argument
$fakeGlobals = array(
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
$request = new SapiRequest($fakeGlobals);
var_dump(
    $request->server === $fakeGlobals['_SERVER'] &&
    $request->input === $fakeGlobals['_POST'] &&
    $request->query === $fakeGlobals['_GET'] &&
    $request->files === $fakeGlobals['_FILES'] &&
    $request->cookie === $fakeGlobals['_COOKIE']
);

// Partial globals argument
$request = new SapiRequest(array(
    '_GET' => array(
        'foo' => 'bar',
    ),
    '_SERVER' => [
        'HTTP_HOST' => 'foo.bar',
    ],
));
var_dump($request->url['host']);
var_dump($request->query['foo']);

// Check for immutability in globals
$_GET['foo'] = new stdClass();
try {
    $request = new SapiRequest($GLOBALS);
    echo 'fail immutable' . PHP_EOL;
} catch( UnexpectedValueException $e ) {
    echo 'ok immutable' . PHP_EOL;
}

// check for references in immutables
$ref = 'ref';
$_GET['ref'] =& $ref;
try {
    $request = new SapiRequest($GLOBALS);
    echo 'fail references' . PHP_EOL;
} catch( UnexpectedValueException $e ) {
    echo 'ok references' . PHP_EOL;
}

// Check __construct can't be called twice
try {
    $request->__construct($GLOBALS);
    echo 'fail reconstruct' . PHP_EOL;
} catch( RuntimeException $e ) {
    echo 'ok reconstruct' . PHP_EOL;
}

--EXPECT--
string(11) "SapiRequest"
bool(true)
string(7) "foo.bar"
string(3) "bar"
ok immutable
ok references
ok reconstruct

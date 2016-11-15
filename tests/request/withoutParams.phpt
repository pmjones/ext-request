--TEST--
ServerRequest::withoutParams
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--GET--
forcecgi
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';

// Named
$request = new ServerRequest();

$params = [
    'foo' => null,
    'bar' => 'baz',
    'dib' => [
        'zim' => 'gir',
    ],
];
$request = $request->withParams($params);

$clone = $request->withoutParams(['bar']);
if( $request === $clone ) echo "fail\n";
if( $clone->params === $request->params ) echo "fail\n";
if( $request->params !== $params ) echo "fail\n";
var_dump($clone->params);

// Reset
$request = new ServerRequest();
$params = [
    'foo' => null,
    'bar' => 'baz',
    'dib' => [
        'zim' => 'gir',
    ],
];
$request = $request->withParams($params);

$clone = $request->withoutParams();
if( $request === $clone ) echo "fail\n";
if( $clone->params === $request->params ) echo "fail\n";
if( $request->params !== $params ) echo "fail\n";
var_dump($clone->params);

--EXPECT--
array(2) {
  ["foo"]=>
  NULL
  ["dib"]=>
  array(1) {
    ["zim"]=>
    string(3) "gir"
  }
}
array(0) {
}
--TEST--
ServerRequest::withParams
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

$request = new ServerRequest();

if( $request === $request->withParams(['foo' => 'bar']) ) echo "fail\n";

$params = [
    'foo' => null,
    'bar' => 'baz',
    'dib' => [
        'zim' => 'gir',
    ],
];
$request = $request->withParams($params);
var_dump($request->params);

try {
    $request = $request->withParams(array('bad' => new stdClass));
} catch( UnexpectedValueException $e ) {
    var_dump($e->getMessage());
}

--EXPECT--
array(3) {
  ["foo"]=>
  NULL
  ["bar"]=>
  string(3) "baz"
  ["dib"]=>
  array(1) {
    ["zim"]=>
    string(3) "gir"
  }
}
string(50) "All $params values must be null, scalar, or array."

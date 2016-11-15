--TEST--
ServerRequest::withParam
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
$expect = [];

if( $request === $request->withParam('foo', 'bar') ) echo "fail\n";

$request = $request->withParam('foo', null);
var_dump($request->params);

$request = $request->withParam('bar', 'baz');
var_dump($request->params);

$request = $request->withParam('dib', ['zim' => 'gir']);
var_dump($request->params);

try {
    $request->withParam('bad', new StdClass());
    echo 'fail';
} catch( UnexpectedValueException $e ) {
    var_dump($e->getMessage());
}

--EXPECT--
array(1) {
  ["foo"]=>
  NULL
}
array(2) {
  ["foo"]=>
  NULL
  ["bar"]=>
  string(3) "baz"
}
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

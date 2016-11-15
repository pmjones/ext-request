--TEST--
ServerRequest::withoutParam
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

$clone = $request->withoutParam('foo');
if( $request === $clone ) echo "fail\n";

$params = [
    'foo' => null,
    'bar' => 'baz',
    'dib' => [
        'zim' => 'gir',
    ],
];
$request = $request->withParams($params);

unset($params['bar']);
$clone = $request->withoutParam('bar');
var_dump($clone->params);
if( $clone->params === $request->params ) echo "fail\n";

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
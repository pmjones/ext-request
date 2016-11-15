--TEST--
ServerRequest::withInput
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

$clone = $request->withInput([]);
var_dump($clone->input);
if( $clone === $request ) echo 'fail';

$clone = $request->withInput(null);
var_dump($clone->input);
if( $clone === $request ) echo 'fail';

$clone = $request->withInput('foo');
var_dump($clone->input);
if( $clone === $request ) echo 'fail';

$clone = $request->withInput(['foo' => 'bar']);
var_dump($clone->input);
if( $clone === $request ) echo 'fail';

try {
    $clone = $request->withInput(new stdClass);
    echo 'fail';
} catch( UnexpectedValueException $e ) {

}

--EXPECT--
array(0) {
}
NULL
string(3) "foo"
array(1) {
  ["foo"]=>
  string(3) "bar"
}

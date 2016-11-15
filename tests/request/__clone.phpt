--TEST--
ServerRequest::__clone
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

$request = new ServerRequest();

$_SERVER['HTTP_HOST'] = 'example.com';
$clone = clone $request;
try {
    $clone->method = 'PATCH';
    echo 'fail';
} catch( Exception $e ) {
    echo 'ok';
}
var_dump($request->url['host']);

--EXPECT--
okstring(9) "localhost"

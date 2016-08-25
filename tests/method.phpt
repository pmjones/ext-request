--TEST--
PhpRequest::$method
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$_SERVER['REQUEST_METHOD'] = 'GET';
$request = new PhpRequest();
echo $request->method, "\n";
$request = new PhpRequest('PUT');
echo $request->method, "\n";
try {
    $request->method = 'PATCH';
    echo 'fail';
} catch( \RuntimeException $e ) {
    echo $e->getMessage();
}
--EXPECT--
GET
PUT
PhpRequest is read-only.
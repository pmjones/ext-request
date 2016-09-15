--TEST--
PhpRequest::$content (cgi)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--POST_RAW--
Content-Type: application/x-www-form-urlencoded
a=1&b=ZYX
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';
$request = new PhpRequest();
var_dump($request->content);
try {
    $request->content = 'foo';
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage(), $request->content);
}
--EXPECT--
string(9) "a=1&b=ZYX"
string(16) "RuntimeException"
string(24) "PhpRequest is read-only."
string(9) "a=1&b=ZYX"
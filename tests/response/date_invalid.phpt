--TEST--
StdResponse::date (invalid)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
try {
    var_dump($response->date(true));
    echo 'fail';
} catch( Exception $e ) {
    echo 'ok';
}
--EXPECT--
ok

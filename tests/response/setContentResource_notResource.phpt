--TEST--
StdResponse::setContentResource (not resource)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new StdResponse();
try {
    $response->setContentResource('not-a-resource', 'disposition');
} catch( TypeError $e ) {
    var_dump($e->getMessage());
}
--EXPECT--
string(97) "Argument 1 passed to StdResponse::setContentResource() must be of the type resource, string given"

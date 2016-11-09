--TEST--
ServerResponse::setContentResource (not resource)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new ServerResponse();
try {
    $response->setContentDownload('not-a-resource', 'disposition');
} catch( TypeError $e ) {
    var_dump($e->getMessage());
}
--EXPECT--
string(100) "Argument 1 passed to ServerResponse::setContentDownload() must be of the type resource, string given"

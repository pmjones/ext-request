--TEST--
ServerResponse::setContentResource (not resource)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new ServerResponse();
try {
    $response->setContentDownload('not-a-resource', 'disposition');
} catch( InvalidArgumentException $e ) {
    var_dump($e->getMessage());
}
--EXPECT--
string(100) "Argument 1 passed to ServerResponse::setContentDownload() must be of the type resource, string given"

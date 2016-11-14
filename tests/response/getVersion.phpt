--TEST--
ServerResponse::getVersion
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->getVersion());
--EXPECT--
string(3) "1.1"

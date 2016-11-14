--TEST--
ServerResponse::setVersion
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
$response->setVersion('1.0');
var_dump($response->getVersion());
--EXPECT--
string(3) "1.0"

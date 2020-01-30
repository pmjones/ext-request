--TEST--
ServerResponse::getCode
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
var_dump($response->getCode());
--EXPECT--
int(200)

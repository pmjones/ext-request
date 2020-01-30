--TEST--
ServerResponse::setCode
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
$response->setCode('500');
var_dump($response->getCode());
$response->setCode(401);
var_dump($response->getCode());
--EXPECT--
int(500)
int(401)

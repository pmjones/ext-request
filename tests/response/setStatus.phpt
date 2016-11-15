--TEST--
ServerResponse::setStatus
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
$response->setStatus('500');
var_dump($response->getStatus());
$response->setStatus(401);
var_dump($response->getStatus());
--EXPECT--
int(500)
int(401)

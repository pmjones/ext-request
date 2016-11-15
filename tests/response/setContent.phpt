--TEST--
ServerResponse::setContent
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
$response->setContent('foo');
var_dump($response->getContent());
--EXPECT--
string(3) "foo"

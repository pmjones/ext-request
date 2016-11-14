--TEST--
ServerResponse::setContentJson
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
$response->setContentJson(array('foo' => 'bar'));
var_dump($response->getContent());
var_dump($response->getHeaders()['content-type']);
--EXPECT--
string(13) "{"foo":"bar"}"
string(16) "application/json"

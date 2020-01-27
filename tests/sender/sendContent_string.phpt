--TEST--
ServerResponse::sendContent (string)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent('foo');
(new ServerResponseSender())->send($response);
--EXPECT--
foo

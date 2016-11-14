--TEST--
ServerResponse::sendContent (resource)
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
$fh = fopen('php://temp', 'w+');
fwrite($fh, 'foo');
$response->setContent($fh);
$response->send();
--EXPECT--
foo

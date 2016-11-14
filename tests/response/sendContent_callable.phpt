--TEST--
ServerResponse::sendContent (callable)
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
$response->setContent(function (ServerResponse $arg) use ($response) {
    if($response !== $arg) echo 'fail';
    echo 'foo';
});
$response->send();
--EXPECT--
foo

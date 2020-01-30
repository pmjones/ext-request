--TEST--
ServerResponse::sendStatus
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
$response->setCode(400);
(new ServerResponseSender())->send($response);
var_dump(headers_list());
var_dump(http_response_code());
--EXPECTHEADERS--
--EXPECT--
array(0) {
}
int(400)

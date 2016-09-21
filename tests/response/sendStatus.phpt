--TEST--
StdResponse::sendStatus
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new StdResponse();
$response->setStatus(400);
$response->send();
var_dump(headers_list());
var_dump(http_response_code());
--EXPECTHEADERS--
--EXPECT--
array(0) {
}
int(400)

--TEST--
ServerResponse::setVersion
--FILE--
<?php
$response = new ServerResponse();
$response->setVersion('1.1');
var_dump($response->getVersion());
--EXPECT--
string(3) "1.1"

--TEST--
ServerResponse::setVersion
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->setVersion('1.1') === $response);
var_dump($response->getVersion());
--EXPECT--
bool(true)
string(3) "1.1"

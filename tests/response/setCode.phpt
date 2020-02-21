--TEST--
ServerResponse::setCode
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->setCode('500') === $response);
var_dump($response->getCode());
$response->setCode(401);
var_dump($response->getCode());
--EXPECT--
bool(true)
int(500)
int(401)

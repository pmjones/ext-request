--TEST--
ServerResponse::getCode
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->getCode());
--EXPECT--
NULL

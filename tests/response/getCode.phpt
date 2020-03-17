--TEST--
SapiResponse::getCode
--FILE--
<?php
$response = new SapiResponse();
var_dump($response->getCode());
--EXPECT--
NULL

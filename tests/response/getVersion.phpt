--TEST--
SapiResponse::getVersion
--FILE--
<?php
$response = new SapiResponse();
var_dump($response->getVersion());
--EXPECT--
NULL

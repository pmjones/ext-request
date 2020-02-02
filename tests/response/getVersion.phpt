--TEST--
ServerResponse::getVersion
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->getVersion());
--EXPECT--
NULL

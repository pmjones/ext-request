--TEST--
ServerResponse::getVersion
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->getVersion());
--EXPECT--
string(3) "1.1"

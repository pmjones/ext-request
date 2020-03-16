--TEST--
SapiResponse::setContent
--FILE--
<?php
$response = new SapiResponse();
var_dump($response->setContent('foo') === $response);
var_dump($response->getContent());
--EXPECT--
bool(true)
string(3) "foo"

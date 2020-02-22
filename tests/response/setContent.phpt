--TEST--
ServerResponse::setContent
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->setContent('foo') === $response);
var_dump($response->getContent());
--EXPECT--
bool(true)
string(3) "foo"

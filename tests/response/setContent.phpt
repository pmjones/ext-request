--TEST--
ServerResponse::setContent
--FILE--
<?php
$response = new ServerResponse();
$response->setContent('foo');
var_dump($response->getContent());
--EXPECT--
string(3) "foo"

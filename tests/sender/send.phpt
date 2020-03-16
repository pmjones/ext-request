--TEST--
SapiResponseSender::sendContent (string)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new SapiResponse();
$response->setContent('foo');
(new SapiResponseSender())->send($response);
--EXPECT--
foo

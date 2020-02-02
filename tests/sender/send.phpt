--TEST--
ServerResponseSender::sendContent (string)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent('foo');
(new ServerResponseSender())->send($response);
--EXPECT--
foo

--TEST--
ServerResponse::sendContent (resource)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$fh = fopen('php://temp', 'w+');
fwrite($fh, 'foo');
$response->setContent($fh);
(new ServerResponseSender())->send($response);
--EXPECT--
foo

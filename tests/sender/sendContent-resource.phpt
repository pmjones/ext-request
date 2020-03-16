--TEST--
SapiResponseSender::sendContent (resource)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new SapiResponse();
$fh = fopen('php://temp', 'w+');
fwrite($fh, 'foo');
$response->setContent($fh);
(new SapiResponseSender())->send($response);
--EXPECT--
foo

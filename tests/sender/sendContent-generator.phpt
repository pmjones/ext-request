--TEST--
SapiResponseSender::sendContent with generator
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new SapiResponse();
$response->setContent(function () {
    yield "foo\n";
    yield "bar\n";
    yield "bat\n";
});
(new SapiResponseSender())->send($response);
--EXPECT--
foo
bar
bat

--TEST--
ServerResponseSender::send with generator
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent(function () {
    yield "foo\n";
    yield "bar\n";
    yield "bat\n";
});
(new ServerResponseSender())->send($response);
--EXPECT--
foo
bar
bat

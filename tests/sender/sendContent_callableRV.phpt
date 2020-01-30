--TEST--
ServerResponse::sendContent (callable return value)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent(function (ServerResponse $arg) use ($response) {
    if($response !== $arg) echo 'fail';
    echo 'foo';
    return 'bar';
});
(new ServerResponseSender())->send($response);
--EXPECT--
foobar

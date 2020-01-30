--TEST--
ServerResponse::sendContent (callable)
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent(function (ServerResponse $arg) use ($response) {
    if($response !== $arg) echo 'fail';
    echo 'foo';
});
(new ServerResponseSender())->send($response);
--EXPECT--
foo

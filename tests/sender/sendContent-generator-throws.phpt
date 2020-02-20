--TEST--
ServerResponseSender::sendContent with generator that throws
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new ServerResponse();
$response->setContent(function () {
    yield "foo\n";
    throw new RuntimeException("failure");
    yield "bar\n";
});
try {
    (new ServerResponseSender())->send($response);
} catch (RuntimeException $e) {
    echo $e->getMessage();
}
--EXPECT--
foo
failure

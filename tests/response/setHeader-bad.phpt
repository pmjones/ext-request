--TEST--
ServerResponse::setHeader-bad
--FILE--
<?php
$response = new ServerResponse();
try {
    $response->setHeader('', 'should-not-show');
} catch (UnexpectedValueException $e) {
    echo $e->getMessage() . PHP_EOL;
}
try {
    $response->setHeader('Should-Not-Show', '');
} catch (UnexpectedValueException $e) {
    echo $e->getMessage() . PHP_EOL;
}
--EXPECTF--
Header label cannot be blank
Header value cannot be blank

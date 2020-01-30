--TEST--
ServerResponse::setHeader_bad
--FILE--
<?php
$response = new ServerResponse();
$response->setHeader('', 'should-error');
$response->setHeader('Should-Error', '');
--EXPECTF--

Warning: ServerResponse::setHeader(): Header label cannot be empty in %s/setHeader_bad.php on line %d

Warning: ServerResponse::setHeader(): Header value cannot be empty in %s/setHeader_bad.php on line %d

--TEST--
ServerResponse::addHeader_bad
--FILE--
<?php
$response = new ServerResponse();
$response->addHeader('', 'should-not-show');
$response->addHeader('Should-Not-Show', '');
--EXPECTF--

Warning: ServerResponse::addHeader(): Header label cannot be empty in %s/addHeader_bad.php on line %d

Warning: ServerResponse::addHeader(): Header value cannot be empty in %s/addHeader_bad.php on line %d
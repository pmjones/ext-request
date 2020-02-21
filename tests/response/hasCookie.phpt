--TEST--
ServerResponse::hasCookie
--FILE--
<?php
$response = new ServerResponse();
$response->setCookie('cookie1', 'v1&%v2');
var_dump($response->hasCookie('cookie1'));
var_dump($response->hasCookie('cookie2'));
--EXPECT--
bool(true)
bool(false)

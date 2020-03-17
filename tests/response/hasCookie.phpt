--TEST--
SapiResponse::hasCookie
--FILE--
<?php
$response = new SapiResponse();
$response->setCookie('cookie1', 'v1&%v2');
var_dump($response->hasCookie('cookie1'));
var_dump($response->hasCookie('cookie2'));
--EXPECT--
bool(true)
bool(false)

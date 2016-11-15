--TEST--
ServerResponse::date
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--INI--
date.timezone=America/Chicago
--FILE--
<?php
$response = new ServerResponse();
var_dump($response->date('1979-11-07 +0000'));
var_dump($response->date(new DateTime('1979-11-07 +0000')));
var_dump($response->date('1979-11-07 03:00:00 +0300'));
--EXPECT--
string(31) "Wed, 07 Nov 1979 00:00:00 +0000"
string(31) "Wed, 07 Nov 1979 00:00:00 +0000"
string(31) "Wed, 07 Nov 1979 00:00:00 +0000"

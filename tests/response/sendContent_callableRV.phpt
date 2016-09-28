--TEST--
StdResponse::sendContent (callable return value)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new StdResponse();
$response->setContent(function () {
    echo 'foo';
    return 'bar';
});
$response->send();
--EXPECT--
foobar

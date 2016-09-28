--TEST--
StdResponse::sendContent (callable)
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--CGI--
--INI--
expose_php=0
--FILE--
<?php
$response = new StdResponse();
$response->setContent(function (StdResponse $arg) use ($response) {
    if($response !== $arg) echo 'fail';
    echo 'foo';
});
$response->send();
--EXPECT--
foo

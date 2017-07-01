--TEST--
request phpinfo
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
phpinfo(INFO_MODULES);
--EXPECTF--
%A
request
%A
Version => %s
%A

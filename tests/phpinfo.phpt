--TEST--
request phpinfo
--FILE--
<?php
phpinfo(INFO_MODULES);
--EXPECTF--
%A
request
%A
Version => %s
%A

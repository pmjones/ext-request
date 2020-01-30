--TEST--
ServerResponse::setCookie_bad
--FILE--
<?php
$response = new ServerResponse();
$response->setCookie('cookie1', 'value1', ['nosuchoption' => true]);

--EXPECTF--

Warning: ServerResponse::setCookie(): Unrecognized key 'nosuchoption' found in the options array in %s/setCookie_bad.php on line %d

Warning: ServerResponse::setCookie(): No valid options were found in the given array in %s/setCookie_bad.php on line %d

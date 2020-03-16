--TEST--
SapiResponse::setCookie-bad
--FILE--
<?php
$response = new SapiResponse();
$response->setCookie('cookie1', 'value1', ['nosuchoption' => true]);
try {
    $response->setCookie('cookie2', 'value2', ['expires' => 1234567890], '/path');
} catch (BadMethodCallException $e) {
    echo $e->getMessage() . PHP_EOL;
}
--EXPECTF--

Warning: SapiResponse::setCookie(): Unrecognized key 'nosuchoption' found in the options array in %ssetCookie-bad.php on line %d

Warning: SapiResponse::setCookie(): No valid options were found in the given array in %ssetCookie-bad.php on line %d
Cannot pass arguments after the options array

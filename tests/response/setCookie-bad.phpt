--TEST--
ServerResponse::setCookie-bad
--FILE--
<?php
$response = new ServerResponse();
$response->setCookie('cookie1', 'value1', ['nosuchoption' => true]);
try {
    $response->setCookie('cookie2', 'value2', ['expires' => 1234567890], '/path');
} catch (BadMethodCallException $e) {
    echo $e->getMessage() . PHP_EOL;
}
--EXPECTF--

Warning: ServerResponse::setCookie(): Unrecognized key 'nosuchoption' found in the options array in %s/setCookie-bad.php on line %d

Warning: ServerResponse::setCookie(): No valid options were found in the given array in %s/setCookie-bad.php on line %d
Cannot pass arguments after the options array

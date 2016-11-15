--TEST--
ServerResponse::date (invalid)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--FILE--
<?php
$response = new ServerResponse();
try {
    var_dump($response->date(true));
    echo 'fail';
} catch( Exception $e ) {
    echo 'ok';
}
--EXPECT--
ok

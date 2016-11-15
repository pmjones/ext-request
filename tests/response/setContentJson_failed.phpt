--TEST--
ServerResponse::setContentJson (failed)
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new ServerResponse();
$value = ['foo' => ['bar' => ['baz' => ['dib' => ['zim' => ['gir']]]]]];
try {
    $response->setContentJson($value, 0, 1);
} catch( RuntimeException $e ) {
    var_dump($e->getMessage());
}
--EXPECT--
string(50) "JSON encoding failed: Maximum stack depth exceeded"

--TEST--
PhpResponse::setContentResource
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--EXTENSIONS--
json
--FILE--
<?php
$response = new PhpResponse();
$fh = fopen('php://temp', 'rb');
$response->setContentResource(
    $fh,
    'whatever',
    $params = ['foo' => 'bar']
);
var_dump($response->getHeaders());
var_dump($fh === $response->getContent());
--EXPECT--
array(3) {
  ["Content-Type"]=>
  array(1) {
    [0]=>
    string(24) "application/octet-stream"
  }
  ["Content-Transfer-Encoding"]=>
  array(1) {
    [0]=>
    string(6) "binary"
  }
  ["Content-Disposition"]=>
  array(1) {
    [0]=>
    string(16) "whatever;foo=bar"
  }
}
bool(true)
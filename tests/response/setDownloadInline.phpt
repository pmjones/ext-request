--TEST--
StdResponse::setDownloadInline
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$response = new StdResponse();
$fh = fopen('php://temp', 'rb');
$response->setDownloadInline($fh, 'foo.txt');
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
    string(25) "inline;filename="foo.txt""
  }
}
bool(true)

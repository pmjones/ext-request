--TEST--
PhpRequest::parseContentType
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
var_dump(PhpRequest::parseContentType('/'));
var_dump(PhpRequest::parseContentType(';'));
var_dump(PhpRequest::parseContentType('text/'));
var_dump(PhpRequest::parseContentType('text/plain'));
var_dump(PhpRequest::parseContentType('text/plain;charset=utf-8'));
var_dump(PhpRequest::parseContentType('text/plain;foo=bar'));
var_dump(PhpRequest::parseContentType('text/plain;foo=bar;charset=utf-8;baz=dib'));
var_dump(PhpRequest::parseContentType('image/x-tiff;baz = "bat"'));
--EXPECT--
NULL
NULL
NULL
array(4) {
  ["value"]=>
  string(10) "text/plain"
  ["type"]=>
  string(4) "text"
  ["subtype"]=>
  string(5) "plain"
  ["params"]=>
  array(0) {
  }
}
array(5) {
  ["value"]=>
  string(10) "text/plain"
  ["type"]=>
  string(4) "text"
  ["subtype"]=>
  string(5) "plain"
  ["charset"]=>
  string(5) "utf-8"
  ["params"]=>
  array(0) {
  }
}
array(4) {
  ["value"]=>
  string(10) "text/plain"
  ["type"]=>
  string(4) "text"
  ["subtype"]=>
  string(5) "plain"
  ["params"]=>
  array(1) {
    ["foo"]=>
    string(3) "bar"
  }
}
array(5) {
  ["value"]=>
  string(10) "text/plain"
  ["type"]=>
  string(4) "text"
  ["subtype"]=>
  string(5) "plain"
  ["charset"]=>
  string(5) "utf-8"
  ["params"]=>
  array(2) {
    ["foo"]=>
    string(3) "bar"
    ["baz"]=>
    string(3) "dib"
  }
}
array(4) {
  ["value"]=>
  string(12) "image/x-tiff"
  ["type"]=>
  string(5) "image"
  ["subtype"]=>
  string(6) "x-tiff"
  ["params"]=>
  array(1) {
    ["baz"]=>
    string(3) "bat"
  }
}
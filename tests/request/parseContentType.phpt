--TEST--
StdRequest::parseContentType
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
try {
    var_dump(StdRequest::parseContentType(null));
} catch( Throwable $e ) {
    var_dump(get_class($e), $e->getMessage());
}
var_dump(StdRequest::parseContentType(''));
var_dump(StdRequest::parseContentType('/'));
var_dump(StdRequest::parseContentType(';'));
var_dump(StdRequest::parseContentType('text/'));
var_dump(StdRequest::parseContentType('text/plain'));
var_dump(StdRequest::parseContentType('text/plain;charset=utf-8'));
var_dump(StdRequest::parseContentType('text/plain;foo=bar'));
var_dump(StdRequest::parseContentType('text/plain;foo=bar;charset=utf-8;baz=dib'));
var_dump(StdRequest::parseContentType('image/x-tiff;baz = "bat"'));
--EXPECT--
string(9) "TypeError"
string(90) "Argument 1 passed to StdRequest::parseContentType() must be of the type string, null given"
NULL
NULL
NULL
NULL
array(2) {
  ["value"]=>
  string(10) "text/plain"
  ["params"]=>
  array(0) {
  }
}
array(3) {
  ["value"]=>
  string(10) "text/plain"
  ["charset"]=>
  string(5) "utf-8"
  ["params"]=>
  array(0) {
  }
}
array(2) {
  ["value"]=>
  string(10) "text/plain"
  ["params"]=>
  array(1) {
    ["foo"]=>
    string(3) "bar"
  }
}
array(3) {
  ["value"]=>
  string(10) "text/plain"
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
array(2) {
  ["value"]=>
  string(12) "image/x-tiff"
  ["params"]=>
  array(1) {
    ["baz"]=>
    string(3) "bat"
  }
}

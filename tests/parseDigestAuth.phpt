--TEST--
PhpRequest::parseDigestAuth
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
var_dump(PhpRequest::parseDigestAuth(null));
var_dump(PhpRequest::parseDigestAuth(''));
var_dump(PhpRequest::parseDigestAuth('nonce="foo",nc=\'bar\',cnonce=baz,qop="dib",username="zim",uri="gir",response="irk"'));
var_dump(PhpRequest::parseDigestAuth(' nonce="foo\\"",nc=\'bar\' ,cnonce= baz , qop="dib",username="zim " , uri="gir" ,response= "irk" '));
var_dump(PhpRequest::parseDigestAuth('nonce="foo",nc=\'bar\',cnonce=baz'));
--EXPECT--
NULL
NULL
object(stdClass)#1 (7) {
  ["nonce"]=>
  string(3) "foo"
  ["nc"]=>
  string(3) "bar"
  ["cnonce"]=>
  string(3) "baz"
  ["qop"]=>
  string(3) "dib"
  ["username"]=>
  string(3) "zim"
  ["uri"]=>
  string(3) "gir"
  ["response"]=>
  string(3) "irk"
}
object(stdClass)#1 (7) {
  ["nonce"]=>
  string(4) "foo""
  ["nc"]=>
  string(3) "bar"
  ["cnonce"]=>
  string(3) "baz"
  ["qop"]=>
  string(3) "dib"
  ["username"]=>
  string(4) "zim "
  ["uri"]=>
  string(3) "gir"
  ["response"]=>
  string(3) "irk"
}
NULL
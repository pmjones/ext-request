--TEST--
ServerRequest::parseDigestAuth
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
try {
    var_dump(ServerRequest::parseDigestAuth(null));
} catch( Throwable $e ) {
    var_dump(get_class($e), $e->getMessage());
}
var_dump(ServerRequest::parseDigestAuth(''));
var_dump(ServerRequest::parseDigestAuth('nonce="foo",nc=\'bar\',cnonce=baz,qop="dib",username="zim",uri="gir",response="irk"'));
var_dump(ServerRequest::parseDigestAuth(' nonce="foo\\"",nc=\'bar\' ,cnonce= baz , qop="dib",username="zim " , uri="gir" ,response= "irk" '));
var_dump(ServerRequest::parseDigestAuth('nonce="foo",nc=\'bar\',cnonce=baz'));
--EXPECTF--
string(9) "TypeError"
string(%d) "Argument 1 passed to ServerRequest::parseDigestAuth() must be of the type string, null given"
NULL
array(7) {
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
array(7) {
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

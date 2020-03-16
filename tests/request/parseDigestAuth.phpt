--TEST--
SapiRequest::parseDigestAuth
--FILE--
<?php
$request = new SapiRequest(['_SERVER' => ['PHP_AUTH_TYPE' => 'Digest', 'PHP_AUTH_DIGEST' => null]]);
var_dump($request->authDigest);

$request = new SapiRequest(['_SERVER' => ['PHP_AUTH_TYPE' => 'Digest', 'PHP_AUTH_DIGEST' => '']]);
var_dump($request->authDigest);

$request = new SapiRequest(['_SERVER' => ['PHP_AUTH_TYPE' => 'Digest', 'PHP_AUTH_DIGEST' => 'nonce="foo",nc=\'bar\',cnonce=baz,qop="dib",username="zim",uri="gir",response="irk"']]);
var_dump($request->authDigest);

$request = new SapiRequest(['_SERVER' => ['PHP_AUTH_TYPE' => 'Digest', 'PHP_AUTH_DIGEST' => ' nonce="foo\\"",nc=\'bar\' ,cnonce= baz , qop="dib",username="zim " , uri="gir" ,response= "irk" ']]);
var_dump($request->authDigest);

$request = new SapiRequest(['_SERVER' => ['PHP_AUTH_TYPE' => 'Digest', 'PHP_AUTH_DIGEST' => 'nonce="foo",nc=\'bar\',cnonce=baz']]);
var_dump($request->authDigest);

--EXPECTF--
NULL
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

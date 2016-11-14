--TEST--
ServerRequest::withUrl
--SKIPIF--
<?php if (
    ! extension_loaded('request')
    && ! getenv('TEST_USERLAND_REQUEST')
) {
    die('skip ');
} ?>
--GET--
forcecgi
--FILE--
<?php
$_SERVER['HTTP_HOST'] = 'localhost';

$request = new ServerRequest();

$clone = $request->withUrl([]);
if( $clone === $request ) echo "fail\n";

$clone = $request->withUrl([
    'scheme' => 'https',
    'host' => 'example.com',
    'port' => 8080,
    'path' => '/foo/bar',
    'query' => 'baz=dib',
]);
if( $clone->url === $request->url ) echo "fail\n";
var_dump($clone->url);

--EXPECT--
array(8) {
  ["scheme"]=>
  string(5) "https"
  ["host"]=>
  string(11) "example.com"
  ["port"]=>
  int(8080)
  ["user"]=>
  NULL
  ["pass"]=>
  NULL
  ["path"]=>
  string(8) "/foo/bar"
  ["query"]=>
  string(7) "baz=dib"
  ["fragment"]=>
  NULL
}

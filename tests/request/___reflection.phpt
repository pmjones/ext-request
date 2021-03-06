--TEST--
SapiRequest reflection (PHP8)
--SKIPIF--
<?php
if( !extension_loaded('request') ) die('skip ');
if( PHP_VERSION_ID < 80000 ) die('skip ');
?>
--FILE--
<?php
echo preg_replace('/\?(\w+)/', '$1 or NULL', (new ReflectionClass(SapiRequest::CLASS)));
var_dump(new SapiRequest([]));
--EXPECT--
Class [ <internal:request> class SapiRequest ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [27] {
    Property [ private $isUnconstructed = true ]
    Property [ public $accept = NULL ]
    Property [ public $acceptCharset = NULL ]
    Property [ public $acceptEncoding = NULL ]
    Property [ public $acceptLanguage = NULL ]
    Property [ public $authDigest = NULL ]
    Property [ public $authPw = NULL ]
    Property [ public $authType = NULL ]
    Property [ public $authUser = NULL ]
    Property [ public $content = NULL ]
    Property [ public $contentCharset = NULL ]
    Property [ public $contentLength = NULL ]
    Property [ public $contentMd5 = NULL ]
    Property [ public $contentType = NULL ]
    Property [ public $cookie = NULL ]
    Property [ public $files = NULL ]
    Property [ public $forwarded = NULL ]
    Property [ public $forwardedFor = NULL ]
    Property [ public $forwardedHost = NULL ]
    Property [ public $forwardedProto = NULL ]
    Property [ public $headers = NULL ]
    Property [ public $input = NULL ]
    Property [ public $method = NULL ]
    Property [ public $query = NULL ]
    Property [ public $server = NULL ]
    Property [ public $uploads = NULL ]
    Property [ public $url = NULL ]
  }

  - Methods [1] {
    Method [ <internal:request, ctor> public method __construct ] {

      - Parameters [2] {
        Parameter #0 [ <required> array $globals ]
        Parameter #1 [ <optional> string or NULL $content = <default> ]
      }
    }
  }
}
object(SapiRequest)#1 (26) {
  ["accept"]=>
  array(0) {
  }
  ["acceptCharset"]=>
  array(0) {
  }
  ["acceptEncoding"]=>
  array(0) {
  }
  ["acceptLanguage"]=>
  array(0) {
  }
  ["authDigest"]=>
  array(0) {
  }
  ["authPw"]=>
  NULL
  ["authType"]=>
  NULL
  ["authUser"]=>
  NULL
  ["content"]=>
  NULL
  ["contentCharset"]=>
  NULL
  ["contentLength"]=>
  NULL
  ["contentMd5"]=>
  NULL
  ["contentType"]=>
  NULL
  ["cookie"]=>
  array(0) {
  }
  ["files"]=>
  array(0) {
  }
  ["forwarded"]=>
  array(0) {
  }
  ["forwardedFor"]=>
  array(0) {
  }
  ["forwardedHost"]=>
  NULL
  ["forwardedProto"]=>
  NULL
  ["headers"]=>
  array(0) {
  }
  ["input"]=>
  array(0) {
  }
  ["method"]=>
  NULL
  ["query"]=>
  array(0) {
  }
  ["server"]=>
  array(0) {
  }
  ["uploads"]=>
  array(0) {
  }
  ["url"]=>
  array(0) {
  }
}

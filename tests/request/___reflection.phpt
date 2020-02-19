--TEST--
ServerRequest reflection
--FILE--
<?php
echo (new ReflectionClass(ServerRequest::CLASS));
var_dump(new ServerRequest([]));
--EXPECT--
Class [ <internal:request> class ServerRequest ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [29] {
    Property [ <default> private $_initialized ]
    Property [ <default> public $accept ]
    Property [ <default> public $acceptCharset ]
    Property [ <default> public $acceptEncoding ]
    Property [ <default> public $acceptLanguage ]
    Property [ <default> public $authDigest ]
    Property [ <default> public $authPw ]
    Property [ <default> public $authType ]
    Property [ <default> public $authUser ]
    Property [ <default> public $content ]
    Property [ <default> public $contentCharset ]
    Property [ <default> public $contentLength ]
    Property [ <default> public $contentMd5 ]
    Property [ <default> public $contentType ]
    Property [ <default> public $cookie ]
    Property [ <default> public $env ]
    Property [ <default> public $files ]
    Property [ <default> public $forwarded ]
    Property [ <default> public $forwardedFor ]
    Property [ <default> public $forwardedHost ]
    Property [ <default> public $forwardedProto ]
    Property [ <default> public $get ]
    Property [ <default> public $headers ]
    Property [ <default> public $input ]
    Property [ <default> public $method ]
    Property [ <default> public $requestedWith ]
    Property [ <default> public $server ]
    Property [ <default> public $uploads ]
    Property [ <default> public $url ]
  }

  - Methods [1] {
    Method [ <internal:request, ctor> public method __construct ] {

      - Parameters [2] {
        Parameter #0 [ <required> array $globals ]
        Parameter #1 [ <optional> string or NULL $content ]
      }
    }
  }
}
object(ServerRequest)#1 (29) {
  ["_initialized":"ServerRequest":private]=>
  bool(true)
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
  ["env"]=>
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
  ["get"]=>
  array(0) {
  }
  ["headers"]=>
  array(0) {
  }
  ["input"]=>
  array(0) {
  }
  ["method"]=>
  NULL
  ["requestedWith"]=>
  NULL
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

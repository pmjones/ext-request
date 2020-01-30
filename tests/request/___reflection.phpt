--TEST--
ServerRequest reflection
--FILE--
<?php
echo (new ReflectionClass(ServerRequest::CLASS));
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
    Property [ <default> public $method ]
    Property [ <default> public $post ]
    Property [ <default> public $requestedWith ]
    Property [ <default> public $server ]
    Property [ <default> public $uploads ]
    Property [ <default> public $url ]
  }

  - Methods [1] {
    Method [ <internal:request, ctor> public method __construct ] {

      - Parameters [1] {
        Parameter #0 [ <required> array $globals ]
      }
    }
  }
}

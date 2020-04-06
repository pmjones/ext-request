--TEST--
SapiResponse reflection
--FILE--
<?php
echo preg_replace('/\?(\w+)/', '$1 or NULL', (new ReflectionClass(SapiResponse::CLASS)));
$response = new SapiResponse();
var_dump($response);
var_dump($response->getHeaders());
--EXPECT--
Class [ <internal:request> class SapiResponse implements SapiResponseInterface ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [6] {
    Property [ <default> private $version ]
    Property [ <default> private $code ]
    Property [ <default> private $headers ]
    Property [ <default> private $cookies ]
    Property [ <default> private $content ]
    Property [ <default> private $callbacks ]
  }

  - Methods [23] {
    Method [ <internal:request, prototype SapiResponseInterface> final public method setVersion ] {

      - Parameters [1] {
        Parameter #0 [ <required> string or NULL $version ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getVersion ] {

      - Parameters [0] {
      }
      - Return [ string or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setCode ] {

      - Parameters [1] {
        Parameter #0 [ <required> int or NULL $code ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getCode ] {

      - Parameters [0] {
      }
      - Return [ int or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setHeader ] {

      - Parameters [2] {
        Parameter #0 [ <required> string $label ]
        Parameter #1 [ <required> string $value ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method addHeader ] {

      - Parameters [2] {
        Parameter #0 [ <required> string $label ]
        Parameter #1 [ <required> string $value ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method unsetHeader ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $label ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getHeader ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $label ]
      }
      - Return [ string or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method hasHeader ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $label ]
      }
      - Return [ bool ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method unsetHeaders ] {

      - Parameters [0] {
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getHeaders ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setCookie ] {

      - Parameters [7] {
        Parameter #0 [ <required> string $name ]
        Parameter #1 [ <optional> string $value ]
        Parameter #2 [ <optional> $expires_or_options ]
        Parameter #3 [ <optional> string $path ]
        Parameter #4 [ <optional> string $domain ]
        Parameter #5 [ <optional> bool $secure ]
        Parameter #6 [ <optional> bool $httponly ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setRawCookie ] {

      - Parameters [7] {
        Parameter #0 [ <required> string $name ]
        Parameter #1 [ <optional> string $value ]
        Parameter #2 [ <optional> $expires_or_options ]
        Parameter #3 [ <optional> string $path ]
        Parameter #4 [ <optional> string $domain ]
        Parameter #5 [ <optional> bool $secure ]
        Parameter #6 [ <optional> bool $httponly ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method unsetCookie ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $name ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method unsetCookies ] {

      - Parameters [0] {
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getCookie ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $name ]
      }
      - Return [ array or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method hasCookie ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $name ]
      }
      - Return [ bool ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getCookies ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setContent ] {

      - Parameters [1] {
        Parameter #0 [ <required> $content ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getContent ] {

      - Parameters [0] {
      }
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method setHeaderCallbacks ] {

      - Parameters [1] {
        Parameter #0 [ <required> array $callbacks ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method addHeaderCallback ] {

      - Parameters [1] {
        Parameter #0 [ <required> callable $callback ]
      }
      - Return [ SapiResponseInterface ]
    }

    Method [ <internal:request, prototype SapiResponseInterface> final public method getHeaderCallbacks ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }
  }
}
object(SapiResponse)#1 (6) {
  ["version":"SapiResponse":private]=>
  NULL
  ["code":"SapiResponse":private]=>
  NULL
  ["headers":"SapiResponse":private]=>
  NULL
  ["cookies":"SapiResponse":private]=>
  NULL
  ["content":"SapiResponse":private]=>
  NULL
  ["callbacks":"SapiResponse":private]=>
  NULL
}
NULL

--TEST--
ServerResponse reflection
--FILE--
<?php
echo preg_replace('/\?(\w+)/', '$1 or NULL', (new ReflectionClass(ServerResponse::CLASS)));
$response = new ServerResponse();
var_dump($response);
var_dump($response->getHeaders());
--EXPECT--
Class [ <internal:request> class ServerResponse implements ServerResponseInterface ] {

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

  - Methods [20] {
    Method [ <internal:request, prototype ServerResponseInterface> final public method setVersion ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $version ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getVersion ] {

      - Parameters [0] {
      }
      - Return [ string or NULL ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setCode ] {

      - Parameters [1] {
        Parameter #0 [ <required> int $code ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getCode ] {

      - Parameters [0] {
      }
      - Return [ int or NULL ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setHeader ] {

      - Parameters [2] {
        Parameter #0 [ <required> string $label ]
        Parameter #1 [ <required> string $value ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method addHeader ] {

      - Parameters [2] {
        Parameter #0 [ <required> string $label ]
        Parameter #1 [ <required> string $value ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method unsetHeader ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $label ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method hasHeader ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $label ]
      }
      - Return [ bool ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method unsetHeaders ] {

      - Parameters [0] {
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getHeaders ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setCookie ] {

      - Parameters [7] {
        Parameter #0 [ <required> string $name ]
        Parameter #1 [ <optional> string $value ]
        Parameter #2 [ <optional> $expires_or_options ]
        Parameter #3 [ <optional> string $path ]
        Parameter #4 [ <optional> string $domain ]
        Parameter #5 [ <optional> bool $secure ]
        Parameter #6 [ <optional> bool $httponly ]
      }
      - Return [ bool ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setRawCookie ] {

      - Parameters [7] {
        Parameter #0 [ <required> string $name ]
        Parameter #1 [ <optional> string $value ]
        Parameter #2 [ <optional> $expires_or_options ]
        Parameter #3 [ <optional> string $path ]
        Parameter #4 [ <optional> string $domain ]
        Parameter #5 [ <optional> bool $secure ]
        Parameter #6 [ <optional> bool $httponly ]
      }
      - Return [ bool ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method unsetCookie ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $name ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method unsetCookies ] {

      - Parameters [0] {
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getCookies ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setContent ] {

      - Parameters [1] {
        Parameter #0 [ <required> $content ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getContent ] {

      - Parameters [0] {
      }
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method setHeaderCallbacks ] {

      - Parameters [1] {
        Parameter #0 [ <required> array $callbacks ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method addHeaderCallback ] {

      - Parameters [1] {
        Parameter #0 [ <required> callable $callback ]
      }
      - Return [ void ]
    }

    Method [ <internal:request, prototype ServerResponseInterface> final public method getHeaderCallbacks ] {

      - Parameters [0] {
      }
      - Return [ array or NULL ]
    }
  }
}
object(ServerResponse)#1 (6) {
  ["version":"ServerResponse":private]=>
  NULL
  ["code":"ServerResponse":private]=>
  NULL
  ["headers":"ServerResponse":private]=>
  NULL
  ["cookies":"ServerResponse":private]=>
  NULL
  ["content":"ServerResponse":private]=>
  NULL
  ["callbacks":"ServerResponse":private]=>
  NULL
}
NULL

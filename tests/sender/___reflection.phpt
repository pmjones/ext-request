--TEST--
SapiResponseSender reflection
--FILE--
<?php
echo (new ReflectionClass(SapiResponseSender::CLASS));
--EXPECT--
Class [ <internal:request> class SapiResponseSender ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [0] {
  }

  - Methods [6] {
    Method [ <internal:request> public method send ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }

    Method [ <internal:request> public method runHeaderCallbacks ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }

    Method [ <internal:request> public method sendStatus ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }

    Method [ <internal:request> public method sendHeaders ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }

    Method [ <internal:request> public method sendCookies ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }

    Method [ <internal:request> public method sendContent ] {

      - Parameters [1] {
        Parameter #0 [ <required> SapiResponseInterface $response ]
      }
      - Return [ void ]
    }
  }
}

--TEST--
SapiUpload reflection
--FILE--
<?php
echo preg_replace('/\?(\w+)/', '$1 or NULL', (new ReflectionClass(SapiUpload::CLASS)));
var_dump(new SapiUpload());
--EXPECT--
Class [ <internal:request> class SapiUpload ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [6] {
    Property [ <default> private $isUninitialized ]
    Property [ <default> public $name ]
    Property [ <default> public $type ]
    Property [ <default> public $size ]
    Property [ <default> public $tmpName ]
    Property [ <default> public $error ]
  }

  - Methods [2] {
    Method [ <internal:request, ctor> public method __construct ] {

      - Parameters [5] {
        Parameter #0 [ <optional> string or NULL $name ]
        Parameter #1 [ <optional> string or NULL $type ]
        Parameter #2 [ <optional> int or NULL $size ]
        Parameter #3 [ <optional> string or NULL $tmpName ]
        Parameter #4 [ <optional> int or NULL $error ]
      }
    }

    Method [ <internal:request> public method move ] {

      - Parameters [1] {
        Parameter #0 [ <required> string $destination ]
      }
    }
  }
}
object(SapiUpload)#1 (5) {
  ["name"]=>
  NULL
  ["type"]=>
  NULL
  ["size"]=>
  NULL
  ["tmpName"]=>
  NULL
  ["error"]=>
  NULL
}

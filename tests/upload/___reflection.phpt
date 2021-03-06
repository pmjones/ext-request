--TEST--
SapiUpload reflection (PHP8)
--SKIPIF--
<?php
if( !extension_loaded('request') ) die('skip ');
if( PHP_VERSION_ID < 80000 ) die('skip ');
?>
--FILE--
<?php
echo preg_replace('/\?(\w+)/', '$1 or NULL', (new ReflectionClass(SapiUpload::CLASS)));
var_dump(new SapiUpload(null, null, null, null, null));
--EXPECT--
Class [ <internal:request> class SapiUpload ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [6] {
    Property [ private $isUnconstructed = true ]
    Property [ public $name = NULL ]
    Property [ public $type = NULL ]
    Property [ public $size = NULL ]
    Property [ public $tmpName = NULL ]
    Property [ public $error = NULL ]
  }

  - Methods [2] {
    Method [ <internal:request, ctor> public method __construct ] {

      - Parameters [5] {
        Parameter #0 [ <optional> string or NULL $name = <default> ]
        Parameter #1 [ <optional> string or NULL $type = <default> ]
        Parameter #2 [ <optional> int or NULL $size = <default> ]
        Parameter #3 [ <optional> string or NULL $tmpName = <default> ]
        Parameter #4 [ <optional> int or NULL $error = <default> ]
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

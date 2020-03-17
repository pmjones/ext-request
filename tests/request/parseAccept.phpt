--TEST--
SapiRequest::parseAccept
--FILE--
<?php
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT' => null,
    ],
]);
var_dump($request->accept);

$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT' => '',
    ],
]);
var_dump($request->accept);

// Accept
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT' => 'application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1',
    ],
]);
var_dump($request->accept);

// Accept-Charset
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT_CHARSET' => 'iso-8859-5;q=0.8, unicode-1-1',
    ],
]);
var_dump($request->acceptCharset);

// Accept-Encoding
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT_ENCODING' => 'compress;q=0.5, gzip;q=1.0',
    ],
]);
var_dump($request->acceptEncoding);

// Accept-Language
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT_LANGUAGE' => 'en-US, en-GB, en, *',
    ],
]);
var_dump($request->acceptLanguage);

// all accepts
$request = new SapiRequest([
    '_SERVER' => [
        'HTTP_ACCEPT' => 'application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1',
        'HTTP_ACCEPT_CHARSET' => 'iso-8859-5;q=0.8, unicode-1-1',
        'HTTP_ACCEPT_ENCODING' => 'compress;q=0.5, gzip;q=1.0',
        'HTTP_ACCEPT_LANGUAGE' => 'en-US, en-GB, en, *',
    ],
]);
var_dump($request->accept);
var_dump($request->acceptCharset);
var_dump($request->acceptEncoding);
var_dump($request->acceptLanguage);
--EXPECTF--
array(0) {
}
array(0) {
}
array(4) {
  [0]=>
  array(3) {
    ["value"]=>
    string(16) "application/json"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(1) {
      ["foo"]=>
      string(3) "bar"
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(15) "application/xml"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  array(3) {
    ["value"]=>
    string(6) "text/*"
    ["quality"]=>
    string(3) "0.2"
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  array(3) {
    ["value"]=>
    string(3) "*/*"
    ["quality"]=>
    string(3) "0.1"
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(11) "unicode-1-1"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(10) "iso-8859-5"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(4) "gzip"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(8) "compress"
    ["quality"]=>
    string(3) "0.5"
    ["params"]=>
    array(0) {
    }
  }
}
array(4) {
  [0]=>
  array(5) {
    ["value"]=>
    string(5) "en-US"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    string(2) "US"
  }
  [1]=>
  array(5) {
    ["value"]=>
    string(5) "en-GB"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    string(2) "GB"
  }
  [2]=>
  array(5) {
    ["value"]=>
    string(2) "en"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    NULL
  }
  [3]=>
  array(5) {
    ["value"]=>
    string(1) "*"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(1) "*"
    ["subtype"]=>
    NULL
  }
}
array(4) {
  [0]=>
  array(3) {
    ["value"]=>
    string(16) "application/json"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(1) {
      ["foo"]=>
      string(3) "bar"
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(15) "application/xml"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  array(3) {
    ["value"]=>
    string(6) "text/*"
    ["quality"]=>
    string(3) "0.2"
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  array(3) {
    ["value"]=>
    string(3) "*/*"
    ["quality"]=>
    string(3) "0.1"
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(11) "unicode-1-1"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(10) "iso-8859-5"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
}
array(2) {
  [0]=>
  array(3) {
    ["value"]=>
    string(4) "gzip"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(3) {
    ["value"]=>
    string(8) "compress"
    ["quality"]=>
    string(3) "0.5"
    ["params"]=>
    array(0) {
    }
  }
}
array(4) {
  [0]=>
  array(5) {
    ["value"]=>
    string(5) "en-US"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    string(2) "US"
  }
  [1]=>
  array(5) {
    ["value"]=>
    string(5) "en-GB"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    string(2) "GB"
  }
  [2]=>
  array(5) {
    ["value"]=>
    string(2) "en"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(2) "en"
    ["subtype"]=>
    NULL
  }
  [3]=>
  array(5) {
    ["value"]=>
    string(1) "*"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
    ["type"]=>
    string(1) "*"
    ["subtype"]=>
    NULL
  }
}
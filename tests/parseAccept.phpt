--TEST--
PhpRequest::parseAccept
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
var_dump(PhpRequest::parseAccept(null));
var_dump(PhpRequest::parseAccept(''));

// Accept
var_dump(PhpRequest::parseAccept('application/xml;q=0.8, application/json;foo=bar, text/*;q=0.2, */*;q=0.1'));

// Accept-Charset
var_dump(PhpRequest::parseAccept('iso-8859-5;q=0.8, unicode-1-1'));

// Accept-Encoding
var_dump(PhpRequest::parseAccept('compress;q=0.5, gzip;q=1.0'));

// Accept-Language
var_dump(PhpRequest::parseAccept('en-US, en-GB, en, *'));
--EXPECTF--
array(0) {
}
array(0) {
}
array(4) {
  [0]=>
  object(stdClass)#%d (3) {
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
  object(stdClass)#%d (3) {
    ["value"]=>
    string(15) "application/xml"
    ["quality"]=>
    string(3) "0.8"
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  object(stdClass)#%d (3) {
    ["value"]=>
    string(6) "text/*"
    ["quality"]=>
    string(3) "0.2"
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  object(stdClass)#%d (3) {
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
  object(stdClass)#%d (3) {
    ["value"]=>
    string(11) "unicode-1-1"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  object(stdClass)#%d (3) {
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
  object(stdClass)#%d (3) {
    ["value"]=>
    string(4) "gzip"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  object(stdClass)#%d (3) {
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
  object(stdClass)#%d (3) {
    ["value"]=>
    string(5) "en-US"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  object(stdClass)#%d (3) {
    ["value"]=>
    string(5) "en-GB"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [2]=>
  object(stdClass)#%d (3) {
    ["value"]=>
    string(2) "en"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  object(stdClass)#%d (3) {
    ["value"]=>
    string(1) "*"
    ["quality"]=>
    string(3) "1.0"
    ["params"]=>
    array(0) {
    }
  }
}
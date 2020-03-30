--TEST--
SapiUpload::__construct
--FILE--
<?php

// Basic construct
$upload = new SapiUpload('foo', 'text/plain', 123, '/tmp/foo.txt', 0);
var_dump(get_class($upload));

// Check __construct can't be called twice
try {
    $upload->__construct(null, null, null, null, null);
    echo 'fail reconstruct' . PHP_EOL;
} catch( RuntimeException $e ) {
    echo 'ok reconstruct' . PHP_EOL;
}

--EXPECT--
string(10) "SapiUpload"
ok reconstruct

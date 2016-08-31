--TEST--
PhpRequest - inheritance
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'REQUEST_METHOD' => 'PUT',
];
class SubPhpRequest extends PhpRequest {
    public $publicTest;
    protected $protectedTest;
    private $privateTest;
    private $magicGetTest;
    public function protectedTest($a) {
        $this->protectedTest = $a;
        return $this->protectedTest;
    }
    public function parentReadOnlyTest() {
        $this->method = 'PATCH';
    }
}
class MagicPhpRequest extends PhpRequest {
    protected $magicTest;
    public function __get($key) {
        return $this->$key;
    }
    public function __set($key, $value) {
        $this->$key = $value;
    }
}
$request = new SubPhpRequest();
var_dump($request->method);
$request->publicTest = 'foo';
var_dump($request->publicTest);
var_dump($request->protectedTest('bar'));
try {
    $request->parentReadOnlyTest();
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
$request = new MagicPhpRequest();
$request->magicTest = 'baz';
var_dump($request->magicTest);
$request->magicTestUndef = 'bat';
var_dump($request->magicTestUndef);
--EXPECT--
string(3) "PUT"
string(3) "foo"
string(3) "bar"
string(16) "RuntimeException"
string(24) "PhpRequest is read-only."
string(3) "baz"
string(3) "bat"

--TEST--
StdRequest - inheritance
--SKIPIF--
<?php if( !extension_loaded('request') ) die('skip '); ?>
--FILE--
<?php
$_SERVER = [
    'HTTP_HOST' => 'example.com',
    'REQUEST_METHOD' => 'PUT',
];
class SubStdRequest extends StdRequest {
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
class MagicStdRequest extends StdRequest {
    protected $magicTest;
    public function __get($key) {
        return $this->$key;
    }
    public function __set($key, $value) {
        $this->$key = $value;
    }
}
$request = new SubStdRequest();
var_dump($request->method);
$request->publicTest = 'foo';
var_dump($request->publicTest);
var_dump($request->protectedTest('bar'));
try {
    $request->parentReadOnlyTest();
} catch( Exception $e ) {
    var_dump(get_class($e), $e->getMessage());
}
$request = new MagicStdRequest();
$request->magicTest = 'baz';
var_dump($request->magicTest);
$request->magicTestUndef = 'bat';
var_dump($request->magicTestUndef);
--EXPECT--
string(3) "PUT"
string(3) "foo"
string(3) "bar"
string(16) "RuntimeException"
string(36) "SubStdRequest::$method is read-only."
string(3) "baz"
string(3) "bat"

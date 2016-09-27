# void http\Env\Request::__construct()

Create an instance of the server's current HTTP request.

Upon construction, the http\Env\Request acquires http\QueryString instances of  query paramters ($\_GET) and form parameters ($\_POST).

It also compiles an array of uploaded files ($\_FILES) more comprehensive than the original $\_FILES array, see http\Env\Request::getFiles() for that matter.

## Params:

None.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

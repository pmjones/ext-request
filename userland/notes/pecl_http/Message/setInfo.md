# http\Message http\Message::setInfo(string $http_info)

Set the complete message info, i.e. type and response resp. request information, at once.
See http\Message::getInfo().

## Params:

* string $http_info  
  The message info (first line of an HTTP message).

## Returns:

* http\Message, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\BadHeaderException

## Format of the message info:

The message info looks similar to the following line for a response, see also http\Message::setResponseCode() and http\Message::setResponseStatus():

    HTTP/1.1 200 Ok

The message info looks similar to the following line for a request, see also http\Message::setRequestMethod() and http\Message::setRequestUrl():

    GET / HTTP/1.1

See http\Message::setHttpVersion().

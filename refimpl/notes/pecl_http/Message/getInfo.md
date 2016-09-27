# string http\Message::getInfo()

Retrieve the first line of a request or response message.
See http\Message::setInfo and also:

* http\Message::getType()
* http\Message::getHttpVersion()
* http\Message::getResponseCode()
* http\Message::getResponseStatus()
* http\Message::getRequestMethod()
* http\Message::getRequestUrl()

## Params:

None.

## Return:

* string, the HTTP message information.
* NULL, if the message is neither of type request nor response.


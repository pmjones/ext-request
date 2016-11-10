# class http\Message implements Countable, Serializable, Iterator

The message class builds the foundation for any request and response message.

See http\Client\Request and http\Client\Response, as well as http\Env\Request and http\Env\Response.

## Constants:

* TYPE_NONE  
  No specific type of message.
* TYPE_REQUEST  
  A request message.
* TYPE_RESPONSE  
  A response message.

## Properties:

* protected int $type = http\Message::TYPE_NONE  
  The message type. See http\Message::TYPE_* constants.
* protected http\Message\Body $body = NULL  
  The message's body.
* protected string $requestMethod = ""  
  The request method if the message is of type request.
* protected string $requestUrl = ""  
  The request url if the message is of type request.
* protected string $responseStatus = ""  
  The respose status phrase if the message is of type response.
* protected int $responseCode = 0  
  The response code if the message is of type response.
* protected string $httpVersion = NULL  
  A custom HTTP protocol version.
* protected array $headers = NULL  
  Any message headers.
* protected http\Message $parentMessage  
  Any parent message.


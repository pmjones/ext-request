# int http\Message\Parser::stream(resource $stream, int $flags, http\Message &$message)

Parse a stream.

## Params:

* stream $resource  
  The message stream to parse from.
* int $flags  
  Any combination of [parser flags](http/Message/Parser#Parser.flags:).
* http\Message &$message  
  The current state of the message parsed.

## Returns:

* int, http\Message\Parser::STATE_* constant.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

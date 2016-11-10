# int http\Message\Parser::parse(string $data, int $flags, http\Message &$message)

Parse a string.

## Params:

* string $data  
  The (part of the) message to parse.
* int $flags  
  Any combination of [parser flags](http/Message/Parser#Parser.flags:).
* http\Message &$message  
  The current state of the message parsed.

## Returns:

* int, http\Message\Parser::STATE_* constant.

## Throws:

* http\Exception\InvalidArgumentException

# http\Message http\Message::prepend(http\Message $message[, bool $top = true])

Prepend message(s) $message to this message, or the top most message of this message chain.

> ***NOTE:***  
> The message chains must not overlap.

## Params:

* http\Message $message  
  The message (chain) to prepend as parent messages.
* Optional bool $top = true  
  Whether to prepend to the top-most parent message.

## Returns:

* http\Message, self.

## Throws:

* http\Exception\InvalidArgumentException
* http\Exception\UnexpectedValueException

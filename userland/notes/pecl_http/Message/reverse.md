# http\Message http\Messae::reverse()

Reverse the message chain and return the former top-most message.

> ***NOTE:***  
> Message chains are ordered in reverse-parsed order by default, i.e. the last parsed message is the message you'll receive from any call parsing HTTP messages.
>
> This call re-orders the messages of the chain and returns the message that was parsed first with any later parsed messages re-parentized.

## Params:

None.

## Returns:

* http\Message, the other end of the message chain.

## Throws:

* http\Exception\InvalidArgumentException

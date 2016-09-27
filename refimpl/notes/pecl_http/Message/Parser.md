# class http\Message\Parser

The parser which is underlying http\Message.

> ***NOTE:****
> This class was added in v2.2.0.

## Constants:

### Parser flags:

* CLEANUP  
  Finish up parser at end of (incomplete) input.
* DUMB_BODIES  
  Soak up the rest of input if no entity length is deducible.
* EMPTY_REDIRECTS  
  Redirect messages do not contain any body despite of indication of such.
* GREEDY  
  Continue parsing while input is available.

### Parser states:

* STATE_FAILURE  
  Parse failure.
* STATE_START  
  Expecting HTTP info (request/response line) or headers.
* STATE_HEADER  
  Parsing headers.
* STATE_HEADER_DONE  
  Completed parsing headers.
* STATE_BODY  
  Parsing the body.
* STATE_BODY_DUMB  
  Soaking up all input as body.
* STATE_BODY_LENGTH  
  Reading body as indicated by `Content-Lenght` or `Content-Range`.
* STATE_BODY_CHUNKED  
  Parsing `chunked` encoded body.
* STATE_BODY_DONE  
  Finished parsing the body.
* STATE_DONE  
  Finished parsing the message.

> ***NOTE:***  
> Most of this states won't be returned to the user, because the parser immediately jumps to the next expected state.

# bool http\Message::isMultipart([string &$boundary = NULL])

Check whether this message is a multipart message based on it's content type.
If the message is a multipart message and a reference $boundary is given, the boundary string of the multipart message will be stored in $boundary.

See http\Message::splitMultipartBody().

## Params:

* Optional reference string &$boundary = NULL  
  A reference where the boundary string will be stored.

## Returns:

* bool, whether this is a message with a multipart "Content-Type".

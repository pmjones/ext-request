# static string http\Env::negotiateLanguage(array $supported[, array &$result)

Negotiate the client's preferred language.

> ***NOTE:***  
> The first elemement of $supported languages serves as a default if no language matches.

## Params:

* array $supported  
  List of supported content languages.
* Optional reference array &$result  
  Out parameter recording negotiation results.
  
## Returns:

* NULL, if negotiation fails.
* string, the negotiated language.

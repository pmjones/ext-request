# static string http\Env::negotiate(string $params, array $supported[, string $prim_typ_sep[, array &$result]])

Generic negotiator. For specific client negotiation see http\Env::negotiateContentType() and related methods.

> ***NOTE:***  
> The first elemement of $supported serves as a default if no operand matches.

## Params:

* string $params  
  HTTP header parameter's value to negotiate.
* array $supported  
  List of supported negotiation operands.
* Optional string $prim_typ_sep  
  A "primary type separator", i.e. that would be a hyphen for content language negotiation (en-US, de-DE, etc.).
* Optional reference array &$result  
  Out parameter recording negotiation results.
  
## Returns:

* NULL, if negotiation fails.
* string, the closest match negotiated, or the default (first entry of $supported).

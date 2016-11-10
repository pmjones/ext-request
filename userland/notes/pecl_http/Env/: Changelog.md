# http\Env Changelog

0. v2.4.0
	* Split off pecl/[apfd](apfd) and pecl/[json_post](json_post)

## Backwards compatibility notes

### Request startup prior v2.4.0

In versions lower than 2.4.0, the http\Env module extends PHP's builtin POST data parser to be run also if the request method is not POST. Additionally it will handle application/json payloads if ext/json is available. Successfully parsed JSON will be put right into the $_POST array.

This functionality has been separated into two distict extensions, pecl/[apfd](apfd) and pecl/[json_post](json_post).

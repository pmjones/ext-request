# Naming

Using "Request" and "Response" likely to conflict with userland non-namespaced classes, a la "Date" (which got changed to "DateTime").

Thus, the prefix "Php". However, that seems redundant, since it is PHP extension in the first place.

Options:

- Go with "Request" and "Response" anyway. This seems rude at best.

- Stick with "Php*" prefix.

- Choose a different class prefix.

    - Not `Http*` -- the name needs to capture the idea that this is *not* strictly an HTTP request and response. The idea is that these represent the server environment, including non-http elements, and the response is from the server to the client.

    - `Server*` is kind of claimed by PSR-7, so probably not an option.

    - `Web*` ?

    - `Ws*` ? (for "web server" or "web SAPI")

- Put them in a namespace -- but then, what namespace? Same naming problem.


# StdRequest additions

- $request->forwarded as array of elements from Forwarded header
- $request->forwarded(For|Host|Port|Proto) from relevant headers
- $request->clientIp as calculated from relevant headers
- $request->remoteAddr

# StdResponse additions

???


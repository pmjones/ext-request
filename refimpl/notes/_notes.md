- $request->forwarded as array of objects

- $request->forwarded(For|Host|Port|Proto) as built from

- $request->remoteAddr as calculated through trusted proxies

- $request->content as php://input.  Lazy-load it, or load at construction time?

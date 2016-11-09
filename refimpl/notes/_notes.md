# ServerRequest additions

- $request->forwarded as array of elements from Forwarded header
- $request->forwarded(For|Host|Port|Proto) from relevant headers

Leave "remote addr" and "client IP" for params. Need proxy info to calculate
client IP reliably anyway.


# ServerResponse additions

???


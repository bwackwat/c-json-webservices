# c-json-webservices
A WIP C webservice supporting JSON.

## Status
* Webserver works.
* It uses sendfile() for common HTTP file requests.
* If the HTTP path is a JSON route, the JSON will be parsed.
* Only single-level JSON objects can be parsed (WIP).
* No database connector (WIP).
* Webserver client tests are pretty cool.
* SQL tests are trashbags, and the setup for Berkeley DB is definitely a struggle WIP.

## Goals
* Gold-plate the JSON serving portion of the code. (Throw away serving HTTP)? 
* Implement TLS.

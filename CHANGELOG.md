# Change Log

All notable changes to this project will be documented in this file.


## [1.0.0b1] - 2016-12-18

### Added

- ServerRequest::$forwarded property, computed from
  $_SERVER['HTTP_FORWARDED']

- ServerRequest::$forwarded(For|Host|Proto), computed from
  $_SERVER['HTTP_X_FORWARDED(FOR|HOST|PROTO)']

### Changed

- Updated docs


## [1.0.0a2] - 2016-11-21

### Changed

- Fixed issue #9 so that ServerRequest::$xhr is computed correctly.

- Updated docs

## [1.0.0a1] - 2016-11-17

Initial release.


* * *

[1.0.0b1]: https://gitlab.com/pmjones/ext-request/tags/1.0.0b1
[1.0.0a2]: https://gitlab.com/pmjones/ext-request/tags/1.0.0a2
[1.0.0a1]: https://gitlab.com/pmjones/ext-request/tags/1.0.0a1

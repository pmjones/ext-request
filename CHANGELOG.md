# Change Log

All notable changes to this project will be documented in this file.

## [2.0.0] - 2020-04-07

Major breaks as a result of declined RFC to include in the standard PHP
distribution.

## [1.0.0b2] - 2017-07-25

### Added

- Support for multiple header callbacks, a la header_register_callback, with
  new methods ServerResponse::setHeaderCallbacks(), addHeaderCallback(), and
  getHeaderCallbacks().

### Changed

- Updated docs and tests.

## [1.0.0b1] - 2016-12-18

### Added

- ServerRequest::$forwarded property, computed from
  `$_SERVER['HTTP_FORWARDED']`

- ServerRequest::$forwarded(For|Host|Proto), computed from
  `$_SERVER['HTTP_X_FORWARDED(FOR|HOST|PROTO)']`

### Changed

- Updated docs


## [1.0.0a2] - 2016-11-21

### Changed

- Fixed issue #9 so that ServerRequest::$xhr is computed correctly.

- Updated docs

## [1.0.0a1] - 2016-11-17

Initial release.


* * *

[2.0.0]: https://github.com/pmjones/ext-request/releases/tag/2.0.0
[1.0.0b2]: https://github.com/pmjones/ext-request/releases/tag/1.0.0b2
[1.0.0b1]: https://github.com/pmjones/ext-request/releases/tag/1.0.0b1
[1.0.0a2]: https://github.com/pmjones/ext-request/releases/tag/1.0.0a2
[1.0.0a1]: https://github.com/pmjones/ext-request/releases/tag/1.0.0a1

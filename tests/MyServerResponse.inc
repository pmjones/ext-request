<?php

class MyServerResponse implements ServerResponseInterface
{
    private $code = 200;
    private $version = '1.0';
    private $headers = [];
    private $cookies = [];
    private $callbacks = [];

    public function setVersion(string $version): ServerResponseInterface {
        $this->version = $version;
        return $this;
    }

    public function getVersion(): string {
        return $this->version;
    }

    public function setCode(int $code): ServerResponseInterface {
        $this->code = $code;
        return $this;
    }

    public function getCode(): int {
        return $this->code;
    }

    public function setHeader(string $label, string $value): ServerResponseInterface {
        $this->headers[strtolower($label)] = $value; // wrong but this is just for testing
        return $this;
    }

    public function addHeader(string $label, string $value): ServerResponseInterface {
        $this->headers[strtolower($label)] = $value; // wrong but this is just for testing
        return $this;
    }

    public function unsetHeader(string $label): ServerResponseInterface {
        unset($this->headers[$label]);
        return $this;
    }

    public function hasHeader(string $label): bool {
        return isset($this->headers[strtolower($label)]);
    }

    public function getHeader(string $label): ?string {
        return $this->headers[strtolower($label)] ?? null;
    }

    public function unsetHeaders(): ServerResponseInterface {
        $this->headers = [];
        return $this;
    }

    public function getHeaders(): array {
        return $this->headers;
    }

    public function setCookie(string $name, string $value = NULL, $expires_or_options = NULL, string $path = NULL, string $domain = NULL, bool $secure = NULL, bool $httponly = NULL): ServerResponseInterface {
        $this->cookies[$name] = [
            'value' => $value,
            'expires' => $expires_or_options,
            'path' => $path,
            'domain' => $domain,
            'secure' => $secure,
            'httponly' => $httponly,
            'url_encode' => true,
        ];
        return $this;
    }

    public function setRawCookie(string $name, string $value = NULL, $expires_or_options = NULL, string $path = NULL, string $domain = NULL, bool $secure = NULL, bool $httponly = NULL): ServerResponseInterface {
        $this->cookies[$name] = [
            'value' => $value,
            'expires' => $expires_or_options,
            'path' => $path,
            'domain' => $domain,
            'secure' => $secure,
            'httponly' => $httponly,
            'url_encode' => true,
        ];
        return $this;
    }

    public function getCookie(string $name): ?array {
        return $this->cookies[$name];
    }

    public function getCookies(): array {
        return $this->cookies;
    }

    public function hasCookie(string $name): bool {
        return isset($this->cookies[$name]);
    }

    public function unsetCookie(string $name): ServerResponseInterface {
        unset($this->cookies[$name]);
        return $this;
    }

    public function unsetCookies(): ServerResponseInterface {
        $this->cookies = [];
        return $this;
    }

    public function setContent($content): ServerResponseInterface {
        $this->content = $content;
        return $this;
    }

    public function getContent() {
        return $this->content;
    }

    public function setHeaderCallbacks(array $callbacks): ServerResponseInterface {
        $this->callbacks = $callbacks;
        return $this;
    }

    public function addHeaderCallback($callback): ServerResponseInterface {
        $this->callbacks[] = $callback;
        return $this;
    }

    public function getHeaderCallbacks(): array {
        return $this->callbacks;
    }
}

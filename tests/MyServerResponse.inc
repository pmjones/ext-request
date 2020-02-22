<?php

class MyServerResponse implements ServerResponseInterface
{
    private $code = 200;
    private $version = '1.0';
    private $headers = [];
    private $cookies = [];
    private $callbacks = [];

    public function setVersion(string $version): void {
        $this->version = $version;
    }

    public function getVersion(): string {
        return $this->version;
    }

    public function setCode(int $code): void {
        $this->code = $code;
    }

    public function getCode(): int {
        return $this->code;
    }

    public function setHeader(string $label, string $value): void {
        $this->headers[strtolower($label)] = $value; // wrong but this is just for testing
    }

    public function addHeader(string $label, string $value): void {
        $this->headers[strtolower($label)] = $value; // wrong but this is just for testing
    }

    public function unsetHeader(string $label): void {
        unset($this->headers[$label]);
    }

    public function hasHeader(string $label): bool {
        return isset($this->headers[strtolower($label)]);
    }

    public function getHeader(string $label): ?string {
        return $this->headers[strtolower($label)] ?? null;
    }

    public function unsetHeaders(): void {
        $this->headers = [];
    }

    public function getHeaders(): array {
        return $this->headers;
    }

    public function setCookie(string $name, string $value = NULL, $expires_or_options = NULL, string $path = NULL, string $domain = NULL, bool $secure = NULL, bool $httponly = NULL): bool {
        $this->cookies[$name] = [
            'value' => $value,
            'expires' => $expires_or_options,
            'path' => $path,
            'domain' => $domain,
            'secure' => $secure,
            'httponly' => $httponly,
            'url_encode' => true,
        ];
        return true;
    }

    public function setRawCookie(string $name, string $value = NULL, $expires_or_options = NULL, string $path = NULL, string $domain = NULL, bool $secure = NULL, bool $httponly = NULL): bool {
        $this->cookies[$name] = [
            'value' => $value,
            'expires' => $expires_or_options,
            'path' => $path,
            'domain' => $domain,
            'secure' => $secure,
            'httponly' => $httponly,
            'url_encode' => true,
        ];
        return true;
    }

    public function getCookies(): array {
        return $this->cookies;
    }

    public function hasCookie(string $name): bool {
        return isset($this->cookies[$name]);
    }

    public function unsetCookie(string $name): void {
        unset($this->cookies[$name]);
    }

    public function unsetCookies(): void {
        $this->cookies = [];
    }

    public function setContent($content): void {
        $this->content = $content;
    }

    public function getContent() {
        return $this->content;
    }

    public function setHeaderCallbacks(array $callbacks): void {
        $this->callbacks = $callbacks;
    }

    public function addHeaderCallback($callback): void {
        $this->callbacks[] = $callback;
    }

    public function getHeaderCallbacks(): array {
        return $this->callbacks;
    }
}

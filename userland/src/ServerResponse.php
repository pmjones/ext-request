<?php
/**
 *
 * Mutable response object.
 *
 */
class ServerResponse
{
    protected $version = '1.1';
    protected $status = 200;
    protected $headers = [];
    protected $cookies = [];
    protected $content;
    protected $callbacks = [];

    public function getVersion() // : string
    {
        return $this->version;
    }

    public function setVersion($version) // : void
    {
        $this->version = $version;
    }

    public function getStatus() // : int
    {
        return $this->status;
    }

    // http_response_code($status)
    public function setStatus($status)
    {
        $this->status = (int) $status;
    }

    public function getHeaders() // : array
    {
        return $this->headers;
    }

    public function getHeader($label) // : string
    {
        $value = '';
        $label = strtolower(trim($label));
        if (isset($this->headers[$label])) {
            $value = $this->headers[$label];
        }
        return $value;
    }

    // header("$label: $value", true);
    public function setHeader($label, $value) // : void
    {
        $label = strtolower(trim($label));
        if (! $label) {
            return;
        }

        if (is_array($value)) {
            $value = $this->csv($value);
        }

        $value = trim($value);
        if (! $value) {
            unset($this->headers[$label]);
            return;
        }

        $this->headers[$label] = $value;
    }

    // header("$label: $value", false);
    public function addHeader($label, $value) // : void
    {
        $label = strtolower(trim($label));
        if (! $label) {
            return;
        }

        if (is_array($value)) {
            $value = $this->csv($value);
        }

        $value = trim($value);
        if (! $value) {
            return;
        }

        if (! isset($this->headers[$label])) {
            $this->headers[$label] = $value;
        } else {
            $this->headers[$label] .= ", {$value}";
        }
    }

    public function getCookies() // : array
    {
        return $this->cookies;
    }

    // setcookie()
    public function setCookie(
        $name,
        $value = "",
        $expire = 0,
        $path = "",
        $domain = "",
        $secure = false,
        $httponly = false
    ) // : void
    {
        $this->cookies[$name] = [
            'raw' => false,
            'value' => (string) $value,
            'expire' => (integer) $expire,
            'path' => (string) $path,
            'domain' => (string) $domain,
            'secure' => (boolean) $secure,
            'httponly' => (boolean) $httponly,
        ];
    }

    // setrawcookie()
    public function setRawCookie(
        $name,
        $value = "",
        $expire = 0,
        $path = "",
        $domain = "",
        $secure = false,
        $httponly = false
    ) // : void
    {
        $this->cookies[$name] = [
            'raw' => true,
            'value' => (string) $value,
            'expire' => (integer) $expire,
            'path' => (string) $path,
            'domain' => (string) $domain,
            'secure' => (boolean) $secure,
            'httponly' => (boolean) $httponly,
        ];
    }

    public function getContent() // : mixed
    {
        return $this->content;
    }

    public function setContent($content) // : void
    {
        $this->content = $content;
    }

    public function setContentJson($value, $options = 0, $depth = 512) // : void
    {
        $content = json_encode($value, $options, $depth);
        if ($content === false) {
            throw new RuntimeException(
                "JSON encoding failed: " . json_last_error_msg(),
                json_last_error()
            );
        }
        $this->setContent($content);
        $this->setHeader('content-type', 'application/json');
    }

    // cf. https://www.iana.org/assignments/cont-disp/cont-disp.xhtml for
    // $disposition and $params values
    public function setContentDownload(
        $fh,
        $name,
        $disposition = 'attachment',
        array $params = []
    ) // : void
    {
        if (! is_resource($fh)) {
            $class = get_class($this);
            throw new InvalidArgumentException("Argument 1 passed to {$class}::setContentDownload() must be of the type resource, string given");
        }

        $params['filename'] = '"' . rawurlencode($name) . '"';
        $disposition .= ';' . $this->semicsv($params);

        $this->setHeader('content-type', 'application/octet-stream');
        $this->setHeader('content-transfer-encoding',  'binary');
        $this->setHeader('content-disposition', $disposition);
        $this->setContent($fh);
    }

    /**
     *
     * Converts any recognizable date format to an RFC 1123 date.
     *
     * @param mixed $date The incoming date value.
     *
     * @return string An RFC 1123 formatted date.
     *
     * @see https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html Section 3.3.1,
     * "servers ... MUST only generate the RFC 1123 format for representing
     * HTTP-date values in header fields."
     */
    public function date($date) // : string
    {
        if ($date instanceof DateTime) {
            $date = clone $date;
        } else {
            $date = new DateTime($date);
        }

        $date->setTimeZone(new DateTimeZone('UTC'));
        return $date->format(DateTime::RFC1123);
    }

    protected function csv(array $values) // : string
    {
        $csv = [];
        foreach ($values as $key => $val) {
            if (is_int($key)) {
                $csv[] = $val;
            } elseif (is_array($val)) {
                $csv[] = $key . ';' . $this->semicsv($val);
            } else {
                $csv[] = "{$key}={$val}";
            }
        }
        return implode(', ', $csv);
    }

    protected function semicsv(array $values) // : string
    {
        $semicsv = [];
        foreach ($values as $key => $val) {
            if (is_int($key)) {
                $semicsv[] = $val;
            } else {
                $semicsv[] = "{$key}={$val}";
            }
        }
        return implode(';', $semicsv);
    }

    public function addHeaderCallback(callable $callback) // : void
    {
        $this->callbacks[] = $callback;
    }

    // header_register_callback($callback)
    //
    // each callback in the array should have the signature
    // `function (ServerResponse $response)` -- returns will be ignored.
    public function setHeaderCallbacks(array $callbacks) // : void
    {
        $this->callbacks = [];
        foreach ($callbacks as $callback) {
            $this->addHeaderCallback($callback);
        }
    }

    public function getHeaderCallbacks() // : array
    {
        return $this->callbacks;
    }

    // if headers_sent() then fail?
    public function send() // : void
    {
        // if headers_sent() then fail?
        $this->runHeaderCallbacks();
        $this->sendStatus();
        $this->sendHeaders();
        $this->sendCookies();
        $this->sendContent();
    }

    protected function runHeaderCallbacks() // : void
    {
        foreach ($this->callbacks as $callback) {
            call_user_func($callback, $this);
        }
    }

    protected function sendStatus() // : void
    {
        header("HTTP/{$this->version} {$this->status}", true, $this->status);
    }

    // capture other cookies at send-time, e.g. session ID?
    protected function sendHeaders() // : void
    {
        foreach ($this->headers as $label => $value) {
            header("{$label}: {$value}", false);
        }
    }

    protected function sendCookies() // : void
    {
        foreach ($this->cookies as $name => $args) {
            if ($args['raw']) {
                setrawcookie(
                    $name,
                    $args['value'],
                    $args['expire'],
                    $args['path'],
                    $args['domain'],
                    $args['secure'],
                    $args['httponly']
                );
            } else {
                setcookie(
                    $name,
                    $args['value'],
                    $args['expire'],
                    $args['path'],
                    $args['domain'],
                    $args['secure'],
                    $args['httponly']
                );
            }
        }
    }

    protected function sendContent() // : void
    {
        if (is_object($this->content) && is_callable($this->content)) {
            $content = call_user_func($this->content, $this);
        } else {
            $content = $this->content;
        }

        if (is_resource($content)) {
            rewind($content);
            fpassthru($content);
            return;
        }

        echo $content;
    }
}

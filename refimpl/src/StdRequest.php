<?php
/**
 *
 * Goals:
 * - Provide a struct of non-session superglobals as read-only properties.
 * - Add other read-only properties calculated from the superglobals ($method,
 *   $headers, $content, etc.) to the struct.
 * - Only build things that don't require application input; e.g., no negotiation,
 *   but build acceptables for application to work through.
 * - No methods, just properties (i.e., a struct).
 *
 * @property-read $acceptCharset
 * @property-read $acceptEncoding
 * @property-read $acceptLanguage
 * @property-read $acceptMedia
 * @property-read $authDigest
 * @property-read $authPw
 * @property-read $authType
 * @property-read $authUser
 * @property-read $content
 * @property-read $contentCharset
 * @property-read $contentLength
 * @property-read $contentMd5
 * @property-read $contentType
 * @property-read $cookie
 * @property-read $env
 * @property-read $files
 * @property-read $get
 * @property-read $headers
 * @property-read $method
 * @property-read $post
 * @property-read $server
 * @property-read $uploads
 * @property-read $url
 * @property-read $xhr
 *
 */
class StdRequest
{
    private $acceptCharset = [];
    private $acceptEncoding = [];
    private $acceptLanguage = [];
    private $acceptMedia = [];
    private $authDigest;
    private $authPw;
    private $authType;
    private $authUser;
    private $content;
    private $contentCharset;
    private $contentLength;
    private $contentMd5;
    private $contentType;
    private $cookie = [];
    private $env = [];
    private $files = [];
    private $get = [];
    private $headers = [];
    private $method = '';
    private $post = [];
    private $server = [];
    private $uploads = [];
    private $url;
    private $xhr = false;

    public function __construct(array $globals = array())
    {
        $this->env = $globals['_ENV'] ?? $_ENV;
        $this->server = $globals['_SERVER'] ?? $_SERVER;

        $this->cookie = $globals['_COOKIE'] ?? $_COOKIE;
        $this->files = $globals['_FILES'] ?? $_FILES;
        $this->get = $globals['_GET'] ?? $_GET;
        $this->post = $globals['_POST'] ?? $_POST;

        $this->setMethod();
        $this->setHeaders();
        $this->setUrl();
        $this->setAccepts();
        $this->setAuth();
        $this->setContent();
        $this->setUploads();
    }

    public function __get($key) // : array
    {
        if (property_exists($this, $key)) {
            return $this->$key;
        }

        $class = get_class($this);
        throw new RuntimeException("{$class}::\${$key} does not exist.");
    }

    public function __set($key, $val) // : void
    {
        $class = get_class($this);
        throw new RuntimeException("{$class}::\${$key} is read-only.");
    }

    public function __isset($key) // : bool
    {
        if (property_exists($this, $key)) {
            return isset($this->$key);
        }

        return false;
    }

    public function __unset($key) // : void
    {
        if( property_exists($this, $key) ) {
            $class = get_class($this);
            throw new RuntimeException("{$class}::\${$key} is read-only.");
        }
    }

    protected function setMethod() // : void
    {
        if (isset($this->server['REQUEST_METHOD'])) {
            $this->method = strtoupper($this->server['REQUEST_METHOD']);
        }

        if ($this->method == 'POST' && isset($this->server['HTTP_X_HTTP_METHOD_OVERRIDE'])) {
            $this->method = strtoupper($this->server['HTTP_X_HTTP_METHOD_OVERRIDE']);
            $this->xhr = true;
        }
    }

    protected function setHeaders() // : void
    {
        // headers prefixed with HTTP_*
        foreach ($this->server as $key => $val) {
            if (substr($key, 0, 5) == 'HTTP_') {
                $key = substr($key, 5);
                $key = str_replace('_', '-', strtolower($key));
                $this->headers[$key] = $val;
            }
        }

        // RFC 3875 headers not prefixed with HTTP_*
        if (isset($this->server['CONTENT_LENGTH'])) {
            $this->headers['content-length'] = $this->server['CONTENT_LENGTH'];
        }

        if (isset($this->server['CONTENT_TYPE'])) {
            $this->headers['content-type'] = $this->server['CONTENT_TYPE'];
        }
    }

    protected function setUrl() // : void
    {
        // scheme
        $scheme = 'http://';
        if (isset($this->server['HTTPS']) && strtolower($this->server['HTTPS']) == 'on') {
            $scheme = 'https://';
        }

        // host
        if (isset($this->server['HTTP_HOST'])) {
            $host = $this->server['HTTP_HOST'];
        } elseif (isset($this->server['SERVER_NAME'])) {
            $host = $this->server['SERVER_NAME'];
        } else {
            throw new RuntimeException("Could not determine host for StdRequest.");
        }

        // port
        preg_match('#\:[0-9]+$#', $host, $matches);
        if ($matches) {
            $host_port = array_pop($matches);
            $host = substr($host, 0, -strlen($host_port));
        }
        $port = isset($this->server['SERVER_PORT'])
            ? ':' . $this->server['SERVER_PORT']
            : '';
        if ($port == '' && ! empty($host_port)) {
            $port = $host_port;
        }

        // all else
        $uri = isset($this->server['REQUEST_URI'])
            ? $this->server['REQUEST_URI']
            : '';

        $url = $scheme . $host . $port . $uri;
        $base =  [
            'scheme' => null,
            'host' => null,
            'port' => null,
            'user' => null,
            'pass' => null,
            'path' => null,
            'query' => null,
            'fragment' => null,
        ];
        $this->url = array_merge($base, parse_url($url));
    }

    protected function setAccepts() // : void
    {
        if (isset($this->headers['accept'])) {
            $this->acceptMedia = $this->parseAccepts($this->headers['accept']);
        }

        if (isset($this->headers['accept-charset'])) {
            $this->acceptCharset = $this->parseAccepts($this->headers['accept-charset']);
        }

        if (isset($this->headers['accept-encoding'])) {
            $this->acceptEncoding = $this->parseAccepts($this->headers['accept-encoding']);
        }

        if (isset($this->headers['accept-language'])) {
            $language = $this->parseAccepts($this->headers['accept-language']);
            foreach ($language as $lang) {
                $parts = explode('-', $lang['value']);
                $lang['type'] = array_shift($parts);
                $lang['subtype'] = array_shift($parts);
                $this->acceptLanguage[] = $lang;
            }
        }
    }

    /**
     *
     * Parses an `Accept*` string into an array.
     *
     * @param string $string An `Accept*` string value; e.g.,
     * `text/plain;q=0.5,text/html,text/*;q=0.1`.
     *
     * @return array
     *
     */
    protected function parseAccepts($string) // : array
    {
        $buckets = [];

        $values = explode(',', $string);
        foreach ($values as $value) {
            $pairs = explode(';', $value);
            $value = $pairs[0];
            unset($pairs[0]);

            $params = array();
            foreach ($pairs as $pair) {
                $param = array();
                preg_match(
                    '/^(?P<name>.+?)=(?P<quoted>"|\')?(?P<value>.*?)(?:\k<quoted>)?$/',
                    $pair,
                    $param
                );
                $params[$param['name']] = $param['value'];
            }

            $quality = '1.0';
            if (isset($params['q'])) {
                $quality = $params['q'];
                unset($params['q']);
            }

            $buckets[$quality][] = [
                'value' => trim($value),
                'quality' => $quality,
                'params' => $params
            ];
        }

        // reverse-sort the buckets so that q=1 is first and q=0 is last,
        // but the values in the buckets stay in the original order.
        krsort($buckets);

        // flatten the buckets back into the return array
        $return = [];
        foreach ($buckets as $q => $accepts) {
            foreach ($accepts as $accept) {
                $return[] = $accept;
            }
        }

        // done
        return $return;
    }

    protected function setAuth() // : void
    {
        if (isset($this->server['PHP_AUTH_PW'])) {
            $this->authPw = $this->server['PHP_AUTH_PW'];
        }

        if (isset($this->server['PHP_AUTH_TYPE'])) {
            $this->authType = $this->server['PHP_AUTH_TYPE'];
        }

        if (isset($this->server['PHP_AUTH_USER'])) {
            $this->authUser = $this->server['PHP_AUTH_USER'];
        }

        if (! isset($this->server['PHP_AUTH_DIGEST'])) {
            return;
        }

        /* modified from https://secure.php.net/manual/en/features.http-auth.php */

        $text = $this->server['PHP_AUTH_DIGEST'];

        $data = [];
        $need = [
            'nonce' => true,
            'nc' => true,
            'cnonce' => true,
            'qop' => true,
            'username' => true,
            'uri' => true,
            'response' => true,
        ];
        $keys = implode('|', array_keys($need));

        preg_match_all(
            '@(' . $keys . ')=(?:([\'"])([^\2]+?)\2|([^\s,]+))@',
            $text,
            $matches,
            PREG_SET_ORDER
        );

        foreach ($matches as $m) {
            $data[$m[1]] = $m[3] ? $m[3] : $m[4];
            unset($need[$m[1]]);
        }

        if (! $need) {
            $this->authDigest = $data;
        }
    }

    protected function setContent() // : void
    {
        $this->content = file_get_contents('php://input');

        if (isset($this->headers['content-md5'])) {
            $this->contentMd5 = $this->headers['content-md5'];
        }

        if (isset($this->headers['content-length'])) {
            $this->contentLength = $this->headers['content-length'];
        }

        if (! isset($this->headers['content-type'])) {
            return;
        }

        $parts = explode(';', $this->headers['content-type']);
        $this->contentType = array_shift($parts);

        if (! $parts) {
            return;
        }

        foreach ($parts as $part) {
            $part = str_replace(' ', '', $part);
            if (substr($part, 0, 8) == 'charset=') {
                $this->contentCharset = substr($part, 8);
                return;
            }
        }
    }

    protected function setUploads() // : void
    {
        foreach ($this->files as $key => $spec) {
            $this->uploads[$key] = $this->setUploadsFromSpec($spec);
        }
    }

    protected function setUploadsFromSpec(array $spec) // : array
    {
        if (is_array($spec['tmp_name'])) {
            return $this->setUploadsFromNested($spec);
        }

        return $spec;
    }

    protected function setUploadsFromNested(array $nested) // : array
    {
        $uploads = [];
        $keys = array_keys($nested['tmp_name']);
        foreach ($keys as $key) {
            $spec = [
                'error'    => $nested['error'][$key],
                'name'     => $nested['name'][$key],
                'size'     => $nested['size'][$key],
                'tmp_name' => $nested['tmp_name'][$key],
                'type'     => $nested['type'][$key],
            ];
            $uploads[$key] = $this->setUploadsFromSpec($spec);
        }
        return $uploads;
    }
}

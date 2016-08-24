<?php
/**
 *
 * Goals:
 * - Provide HTTP-related superglobals as read-only instance properties.
 * - Add $method for the HTTP method, and convenience methods for is*().
 * - Add $headers for normalized HTTP headers
 * - Only build things that don't require application input; e.g., no negotiation,
 *   but build acceptables for application to work through.
 *
 * It looks like we can have $body as a scalar, null, or array, but not as an
 * object. Maybe scan through arrays to see if their top-level members are
 * objects other than StdClass, otherwise you can use methods to modify objects
 * in the array.
 *
 * @property-read $acceptCharset
 * @property-read $acceptEncoding
 * @property-read $acceptLanguage
 * @property-read $acceptMedia
 * @property-read $authDigest
 * @property-read $authPw
 * @property-read $authType
 * @property-read $authUser
 * @property-read $cookie
 * @property-read $env
 * @property-read $files
 * @property-read $get
 * @property-read $headers
 * @property-read $method
 * @property-read $post
 * @property-read $secure
 * @property-read $server
 * @property-read $url
 * @property-read $xhr
 *
 */
class PhpRequest
{
    protected $acceptCharset = [];
    protected $acceptEncoding = [];
    protected $acceptLanguage = [];
    protected $acceptMedia = [];
    protected $authDigest;
    protected $authPw;
    protected $authType;
    protected $authUser;
    protected $cookie = [];
    protected $env = [];
    protected $files = [];
    protected $get = [];
    protected $headers = [];
    protected $method = '';
    protected $post = [];
    protected $secure = false;
    protected $server = [];
    protected $url;

    protected $xhr = false;

    public function __construct($method = '')
    {
        $this->env = $_ENV;
        $this->server = $_SERVER;

        $this->cookie = $_COOKIE;
        $this->files = $_FILES;
        $this->get = $_GET;
        $this->post = $_POST;

        $this->setMethod($method);
        $this->setHeaders();
        $this->setSecure();
        $this->setUrl();
        $this->setAccepts();
        $this->setAuth();
    }

    public function __get($key) // : array
    {
        if (property_exists($this, $key)) {
            return $this->$key;
        }

        throw new RuntimeException("PhpRequest::\${$key} does not exist.");
    }

    public function __set($key, $val) // : void
    {
        throw new RuntimeException("PhpRequest is read-only.");
    }

    public function __isset($key) // : bool
    {
        if (property_exists($this, $key)) {
            return isset($this->$key);
        }

        throw new RuntimeException("PhpRequest::\${$key} does not exist.");
    }

    public function __unset($key) // : void
    {
        throw new RuntimeException("PhpRequest is read-only.");
    }

    protected function setMethod($method = '') // : void
    {
        // force the method?
        if ($method != '') {
            $this->method = strtoupper($method);
            return;
        }

        // determine method from request
        if (isset($this->server['REQUEST_METHOD'])) {
            $this->method = strtoupper($this->server['REQUEST_METHOD']);
        }

        // XmlHttpRequest method override?
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
                $key = ucwords($key, '-');
                $this->headers[$key] = $val;
            }
        }

        // RFC 3875 headers not prefixed with HTTP_*
        if (isset($this->server['CONTENT_LENGTH'])) {
            $this->headers['Content-Length'] = $this->server['CONTENT_LENGTH'];
        }

        if (isset($this->server['CONTENT_TYPE'])) {
            $this->headers['Content-Type'] = $this->server['CONTENT_TYPE'];
        }
    }

    protected function setSecure()
    {
        $scheme = isset($this->server['HTTPS'])
            && strtolower($this->server['HTTPS']) == 'on';

        $port = isset($this->server['SERVER_PORT'])
            && $this->server['SERVER_PORT'] == 443;

        $forward = isset($this->server['HTTP_X_FORWARDED_PROTO'])
            && strtolower($this->server['HTTP_X_FORWARDED_PROTO']) == 'https';

        $this->secure = $scheme || $port || $forward;
    }

    protected function setUrl()
    {
        // scheme
        $scheme = ($this->secure) ? 'https://' : 'http://';

        // host
        if (isset($this->server['HTTP_HOST'])) {
            $host = $this->server['HTTP_HOST'];
        } elseif (isset($this->server['SERVER_NAME'])) {
            $host = $this->server['SERVER_NAME'];
        } else {
            throw new RuntimeException("Could not determine host for PhpRequest.");
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
        $this->url = (object) array_merge($base, parse_url($url));
    }

    protected function setAccepts() // : void
    {
        if (isset($this->headers['Accept'])) {
            $this->acceptMedia = $this->parseAccepts($this->headers['Accept']);
        }

        if (isset($this->headers['Accept-Charset'])) {
            $this->acceptCharset = $this->parseAccepts($this->headers['Accept-Charset']);
        }

        if (isset($this->headers['Accept-Encoding'])) {
            $this->acceptEncoding = $this->parseAccepts($this->headers['Accept-Encoding']);
        }

        if (isset($this->headers['Accept-Language'])) {
            $language = $this->parseAccepts($this->headers['Accept-Language']);
            foreach ($language as $lang) {
                $parts = explode('-', $lang->value);
                $lang->type = array_shift($parts);
                $lang->subtype = array_shift($parts);
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
                $return[] = (object) $accept;
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
            $this->authDigest = (object) $data;
        }
    }
}

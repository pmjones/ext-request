
# vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab

PHP_ARG_ENABLE(request, whether to enable request support,
[  --enable-request     Enable request support])

# MAIN -------------------------------------------------------------------------
if test "$PHP_REQUEST" != "no"; then
    PHP_NEW_EXTENSION(request, php_request.c request_utils.c request_parsers.c stdresponse.c stdrequest.c, $ext_shared, , $PHP_REQUEST_FLAGS)
    PHP_ADD_MAKEFILE_FRAGMENT
fi

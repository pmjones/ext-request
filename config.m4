
# vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab

PHP_ARG_ENABLE(request, whether to enable request support,
[  --enable-request     Enable request support])

# MAIN -------------------------------------------------------------------------
if test "$REQUEST" != "no"; then
    PHP_NEW_EXTENSION(request, php_request.c parsers.c request_sapiupload.c, $ext_shared, , $REQUEST_FLAGS)
    PHP_ADD_MAKEFILE_FRAGMENT
    PHP_INSTALL_HEADERS([ext/request], [php_request.h])
fi

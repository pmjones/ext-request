
# vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab

PHP_ARG_ENABLE(request, whether to enable request support,
[  --enable-request     Enable request support])

# MAIN -------------------------------------------------------------------------
if test "$PHP_REQUEST" != "no"; then
    PHP_NEW_EXTENSION(request, request.c request_utils.c, $ext_shared, , $PHP_REQUEST_FLAGS)
fi

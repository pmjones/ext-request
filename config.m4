
# vim: tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab

PHP_ARG_ENABLE(request, whether to enable request support,
[  --enable-request     Enable request support])

# MAIN -------------------------------------------------------------------------
if test "$REQUEST" != "no"; then
    PHP_NEW_EXTENSION(request, request.c parsers.c serverresponse.c serverrequest.c, $ext_shared, , $REQUEST_FLAGS)
    PHP_ADD_MAKEFILE_FRAGMENT
fi

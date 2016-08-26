
$(srcdir)/request_scanners.c: $(srcdir)/request_scanners.re
	re2c request_scanners.re -o request_scanners.c

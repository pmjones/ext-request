
$(srcdir)/request_scanners.c: $(srcdir)/request_scanners.re
	(cd $(top_srcdir); $(RE2C) --no-generation-date -o request_scanners.c request_scanners.re)

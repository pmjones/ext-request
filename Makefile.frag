
$(srcdir)/request_parsers.c: $(srcdir)/request_parsers.re
	(cd $(top_srcdir); $(RE2C) --no-generation-date -o request_parsers.c request_parsers.re)

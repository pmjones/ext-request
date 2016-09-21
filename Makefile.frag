
$(srcdir)/parsers.c: $(srcdir)/parsers.re
	(cd $(top_srcdir); $(RE2C) --no-generation-date -o parsers.c parsers.re)

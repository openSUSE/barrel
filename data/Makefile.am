#
# Makefile.am for barrel/data
#

EXTRA_DIST = barrel.logrotate

install-data-local:
	install -d -m 755 $(DESTDIR)/etc/barrel
	install -D -m 644 barrel.logrotate $(DESTDIR)/etc/logrotate.d/barrel


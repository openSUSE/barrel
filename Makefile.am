#
# Makefile.am for barrel
#

SUBDIRS = barrel data doc testsuite po

AUTOMAKE_OPTIONS = foreign dist-xz no-dist-gzip subdir-objects

doc_DATA = AUTHORS LICENSE

EXTRA_DIST = $(doc_DATA) VERSION

barrel-$(VERSION).tar.xz: dist-xz

package: barrel-$(VERSION).tar.xz
	rm -f package/barrel-*.tar.xz
	mv barrel-$(VERSION).tar.xz package/

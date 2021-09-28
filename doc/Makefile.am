#
# Makefile.am for barrel/doc
#

man_MANS = barrel.8

.xml.8: .xml
	$(XSLTPROC) --nonet manpages.xsl $<

html: $(man_MANS:%.8=%.html)

.xml.html: .xml
	$(XSLTPROC) --nonet html.xsl $< > $@

EXTRA_DIST = manpages.xsl html.xsl

clean-local:
	rm -f *.8 *.html


#
# Makefile.am for barrel/po
#

XGETTEXT = xgettext
MSGFMT = msgfmt
MSGMERGE = msgmerge

SRCFILES = $(wildcard ../barrel/*.cc ../barrel/*/*.cc)

POFILES = $(wildcard *.po)

MOFILES = $(POFILES:.po=.mo)

all: $(MOFILES)

$(PACKAGE).pot: $(SRCFILES)
	$(XGETTEXT) --sort-output --add-comments=TRANSLATORS: --no-location \
	    --keyword=_ --keyword=_:1,2 --foreign-user \
	    --copyright-holder="SUSE LLC" \
	    --default-domain=$(PACKAGE) --output=$(PACKAGE).pot $(SRCFILES)

%.mo: %.po
	$(MSGFMT) -c -v --statistics -o $@ $<

merge: $(PACKAGE).pot
	files='$(POFILES)'; \
	for file in $$files; do \
	    base=`basename $$file`; \
	    echo "$(MSGMERGE) --update $$base $(PACKAGE).pot"; \
	    $(MSGMERGE) --update $$base $(PACKAGE).pot; \
	done

install-data-local: $(MOFILES)
	files='$(MOFILES)'; \
	for file in $$files; do \
	    base=`basename $$file`; \
	    lang=`echo $$base | sed 's/\.mo$$//'`; \
	    langdir=$(localedir)/$$lang/LC_MESSAGES; \
	    $(mkinstalldirs) -d $(DESTDIR)$$langdir; \
	    echo " $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$$langdir/$(PACKAGE).mo"; \
	    $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$$langdir/$(PACKAGE).mo; \
	done

CLEANFILES = $(MOFILES)

EXTRA_DIST = $(POFILES)

TANGLED := relay.symon
DOC := relay.pdf relay.html README.md

all: relay.symon

$(TANGLED): tangle

tangle: relay.org
	emacs --batch --eval "(require 'org)" --eval '(org-babel-tangle-file "$<")'

doc: $(DOC)

README.md: README.org
	pandoc -s $< -t gfm -o $@

README.org: relay.org.org
	mv $< $@

%.org.org: %.org
	emacs $< --batch  --eval "(require 'org)" --eval "(require 'ox-md)" --eval '(org-org-export-to-org)'

%.html: %.org
	emacs $< --batch  --eval "(require 'org)" --eval "(require 'ox-md)" --eval '(org-html-export-to-html)'

%.pdf: %.org
	emacs $< --batch  --eval "(require 'org)" --eval "(require 'ox-latex)" --eval '(org-latex-export-to-pdf)'

clean:
	$(RM) $(TANGLED) $(DOC)

.PHONY: all clean doc tangle

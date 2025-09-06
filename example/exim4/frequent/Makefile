TANGLED := frequent.symon
DOC := frequent.pdf frequent.html README.md

all: frequent.symon

$(TANGLED): tangle

tangle: frequent.org
	emacs --batch --eval "(require 'org)" --eval '(org-babel-tangle-file "$<")'

doc: $(DOC)

README.md: README.org
	pandoc -s $< -t gfm -o $@

README.org: frequent.org.org
	mv $< $@

%.org.org: %.org
	emacs $< --batch  --eval "(require 'org)" --eval '(org-org-export-to-org)'

%.html: %.org
	emacs $< --batch  --eval "(require 'org)" --eval '(org-html-export-to-html)'

%.pdf: %.org
	emacs $< --batch  --eval "(require 'org)" --eval "(require 'ox-latex)" --eval '(org-latex-export-to-pdf)'

clean:
	$(RM) $(TANGLED) $(DOC)

.PHONY: all clean doc tangle

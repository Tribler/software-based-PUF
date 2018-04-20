all:   clean pdf
 
pdf:   dvi
	@dvipdf thesis.dvi
 
spell: *.tex
	@for file in $?; do aspell --lang=en_GB --mode=tex -c $$file; done
 
dvi:
	@latex thesis.tex
	# @if grep -q '\\citation' *.aux ; then bibtex thesis ; fi
	bibtex thesis
	@latex thesis.tex
	@latex thesis.tex
	@latex thesis.tex
 
clean:
	@-rm -f *.aux *.log *.bak *.dvi *.pdf thesis.toc thesis.bbl thesis.blg
 
.PHONY: clean,spell,dvi,pdf

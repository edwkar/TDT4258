LATEX = pdflatex
BIBTEX = bibtex
LATEXOPTIONS = --halt-on-error
DOT = dot

ILLUSTRATIONS = modules.dot.png main-logic.dot.png dac.dot.png
REFS = refs.bib

TARGET = report.pdf

$(TARGET): report.tex $(REFS) $(ILLUSTRATIONS)
	$(LATEX) $(LATEXOPTIONS) $<
	$(BIBTEX) report.aux
	$(LATEX) $(LATEXOPTIONS) $<
	$(LATEX) $(LATEXOPTIONS) $<

%.dot.png: %.dot
	$(DOT) -Tpng $< -o $@

.PHONY: clean
clean:
	rm -fr *.pdf *.aux *.log *.bbl *.blg $(ILLUSTRATIONS)

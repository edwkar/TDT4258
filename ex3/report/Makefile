LATEX = pdflatex
BIBTEX = bibtex
LATEXOPTIONS = --halt-on-error -shell-escape
DOT = dot

ILLUSTRATIONS = scorched-main-logic.dot.png

TARGET = report.pdf

$(TARGET): report.tex $(ILLUSTRATIONS)
	$(LATEX) $(LATEXOPTIONS) $<
	$(BIBTEX) report.aux
	$(LATEX) $(LATEXOPTIONS) $<
	$(LATEX) $(LATEXOPTIONS) $<

%.dot.png: %.dot
	$(DOT) -Tpng $< -o $@

.PHONY: clean
clean:
	rm -fr *.pdf *.aux *.log *.bbl *.blg $(ILLUSTRATIONS)

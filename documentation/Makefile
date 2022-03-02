MAIN_TEX_FILE= paper.tex

all: pdf

pdf: 
	xelatex --interaction=batchmode $(MAIN_TEX_FILE)

clean:
	@rm *.log *.aux ./chapters/*.log

.PHONY : all pdf clean

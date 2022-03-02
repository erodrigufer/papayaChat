MAIN_TEX_FILE = paper.tex
OUTPUT_FILE = paper.pdf

all: pdf

pdf: 
	@xelatex --interaction=batchmode $(MAIN_TEX_FILE) > /dev/null 2>&1 || echo "pdf generation done!"
	@open -g $(OUTPUT_FILE)

clean:
	@rm *.log *.aux ./chapters/*.log

.PHONY : all pdf clean

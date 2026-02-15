TARGET = IR

all: $(TARGET).svg

# One command for generating the SVG
$(TARGET).svg: $(TARGET).tex
	latex $(TARGET).tex
	dvisvgm $(TARGET).dvi

# Cleanup
clean:
	rm -f *.aux *.log *.dvi

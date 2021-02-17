
QCDIR ?= $(HOME)/alice/QualityControl

run: trddigits.root
	o2-trd-trap-sim \
	| o2-qc o2-qc --config json://$(QCDIR)/Modules/TRD/DigitsTask.json

web:
	open https://qcg-test.cern.ch/?page=objectTree

clean:
	rm -f padrow_*.pdf *log *log0
	rm -f dpl-config.json imgui.ini

trddigits.root: ../100ev/trddigits.root
	ln -sf $< $@

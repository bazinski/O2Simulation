
NEVENTS ?= 10

all: sim ana

sim: foo/trddigits.root

ana: foo/o2sim_HitsTRD.root
	cd $(dir $<); root -b -q -l '../CheckHits.C()'


.PRECIOUS: %/o2sim_HitsTRD.root
%/o2sim_HitsTRD.root:
	mkdir -p $(dir $@)
	cd $(dir $@); o2-sim -n $(NEVENTS)

%/trddigits.root: %/o2sim_HitsTRD.root
	cd $(dir $@); o2-sim-digitizer-workflow --onlyDet TRD

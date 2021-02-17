
draw: trddigits.root o2sim_HitsTRD.root draw.C
	root ../draw.C

.PRECIOUS: o2sim_grp.root o2sim_HitsTRD.root trddigits.root
o2sim_grp.root:
	o2-sim -n 1

o2sim_HitsTRD.root: fakeHits.C o2sim_grp.root
	root -b -q -l fakeHits.C

trddigits.root: o2sim_HitsTRD.root
	o2-sim-digitizer-workflow -b --onlyDet TRD

clean::
	rm -f *.root AutoDict*

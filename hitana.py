#!/usr/bin/env python3


import ROOT


def hitana(filename):
    tf = ROOT.TFile(filename)
    print(dir(tf))
    tree = tf.Get("o2sim")


    #  = tf.GetObject("o2sim")
    print(dir(fo))
    # for x in fo:
    #     print(x)
    # import pdb;pdb.set_trace()


if __name__ == '__main__':
    hitana("foo/o2sim_HitsTRD.root")

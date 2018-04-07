#!/usr/binenv python

import ROOT
import root.utils as root

f = open('whatever2.txt')

g = ROOT.TGraphAsymmErrors()

i = 0
for line in f:
    fields = line.split()
    print fields
    g.SetPoint(i, float(fields[0]), float(fields[1]))
    g.SetPointError(i, float(fields[3]), float(fields[2]), float(fields[5]), float(fields[4]))
    i += 1

c = ROOT.TCanvas()
frame = root.DrawFrame(g);
g.Draw('p')

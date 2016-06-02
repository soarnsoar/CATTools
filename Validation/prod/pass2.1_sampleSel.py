#!/usr/bin/env python

import sys, os
if not os.path.exists("pass2"): os.mkdir("pass2")

from math import sqrt
import json
from ROOT import *
def findHists(d, hists):
    if d == None: return
    for name in [key.GetName() for key in d.GetListOfKeys()]:
        obj = d.Get(name)
        if obj == None: continue
        if obj.IsA().InheritsFrom("TDirectory"):
            findHists(obj, hists)
        else:
            hPath = (d.GetPath().split(':')[-1])+"/"+obj.GetName()
            hists.add(hPath[1:])

ds = {}
dsIn = json.loads(open("pass1/dataset.json").read())
hists = {}
for name in dsIn:
    x = dsIn[name]
    title = x["title"]
    stype = x["type"]
    safeTitle = title
    for i in " ;:!@#$%^&*()-+=/<>?[]{}|": safeTitle = safeTitle.replace(i,'_')
    if title not in ds:
        ds[title] = {
            'colour':x['colour'],
            'hist':'pass2/central/%s.root' % safeTitle, ## Path to the merged histogram
            'samples':[], ## List of input samples
        }

    ds[title]['samples'].append({
        'type':stype,
        'avgWgt':x['avgWgt'], 'nevt':x['nevt'],
        'xsec':x['xsec'], 'lumi':x['lumi'], 'normFactor':x['normFactor'],
        'hist':x['hist'],
    })

    sstype = stype.split('/')[0]
    if sstype not in hists or len(hists[sstype]) == 0:
        print "Reading histogram contents of sample type", sstype
        hset = set()
        f = TFile(x['hist'])
        findHists(f, hset)
        f.Close()
        hists[sstype] = sorted(list(hset))

open("pass2/dataset.json", "w").write(json.dumps(ds, sort_keys=True, indent=4))
open("pass2/hists.json", "w").write(json.dumps(hists, sort_keys=True, indent=4))


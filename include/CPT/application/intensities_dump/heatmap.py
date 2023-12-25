#!/usr/bin/python3

import matplotlib
matplotlib.use('tkagg')
# matplotlib.rcParams['toolbar'] = 'None'

import matplotlib.pyplot as pt
pt.ion()

import sys, re, math
import numpy as np
from glob import glob

def otsu_thres(x, nbins, arange): # x vectorize image, nbins( 2^14 ), arange ( 0 ~ 65536 )
    P, X = np.histogram(x, bins = nbins, range = arange)
    X.resize(len(P))
    res = []
    for i in range(5+1, len(X)-5):
        N1 = np.sum(P[:i]) + 1e-8
        N2 = np.sum(P[i:]) + 1e-8
        u1 = X[:i].dot(P[:i]) / N1
        u2 = X[i:].dot(P[i:]) / N2
        s1 = np.sum(((X[:i] - u1) ** 2).dot(P[:i]) / N1)
        s2 = np.sum(((X[i:] - u2) ** 2).dot(P[i:]) / N2)
        # print(i, N1, N2, u1, u2, s1 + s2, sep = '\t')
        res.append(s1 + s2)
    i = np.argmin(res)
    N1 = P[:i].sum() + 1e-8
    N2 = P[i:].sum() + 1e-8
    u1 = X[:i].dot(P[:i]) / N1
    u2 = X[i:].dot(P[i:]) / N2
    s1 = np.sum(((X[:i] - u1) ** 2).dot(P[:i]) / N1)
    s2 = np.sum(((X[i:] - u2) ** 2).dot(P[i:]) / N2)
    return X[i], u1, u2, (u1 - 2 * math.sqrt(s1)), (u2 + 8 * math.sqrt(s2)) # threshold, pre mean, pos mean

def heatmap(fname):
    x = np.loadtxt(fname)

    # pt.figure(2, figsize = (16, 10))
    # pt.clf()
    # pt.hist(np.log1p(x.ravel()), bins = 2048, range = (0, np.log1p(65536)))
    # pt.tight_layout(pad = 3.0)
    # pt.show()

    z = np.log2(x.ravel() + 1e-16)
    # z = x.ravel()
    m = np.median(z)
    d = 1.4826 * np.median(np.abs(z - m))
    lo = max(m - 3 * d, 0.0)
    up = min(m + 3 * d, np.log2(65535.0))
    # up = min(m + 3 * d, 65535.0)
    rng = np.where(np.logical_and(z >= lo, z < up))
    thres, u1, u2, lo, up = otsu_thres(z[rng], 2 ** 12, (lo, up))
    # pt.xlabel('thres = {0}'.format(thres))
    thres = 2.0 ** thres
    u1    = 2.0 ** u1
    u2    = 2.0 ** u2
    lo    = max(2.0 ** lo, 0)
    up    = min(2.0 ** up, 65535)
    # lo = max(lo, 0)
    # up = min(up, 65535.0)
    print(lo, u1, thres, u2, up)

    pt.figure(1, figsize = (16, 10))
    pt.clf()
    pt.imshow(x, interpolation = 'none', cmap = pt.cm.coolwarm)
    pt.clim(thres, up)
    pt.colorbar()
    pt.title(fname)
    pt.xlabel('shape = [ {0} x {1} ]'.format(x.shape[0], x.shape[1]))
    pt.tight_layout(pad = 3.0)
    pt.savefig(re.sub(r'.mat', '.png', fname))

if (__name__ == '__main__'):
    for item in sys.argv[1:]:
        print('plot {0}'.format(item))
        heatmap(item)


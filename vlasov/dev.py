from __future__ import print_function

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

import sys, os

import pyplasmaDev as pdev


class conf:

    outdir = "out"

    Nxv = 5
    Nyv = 10
    Nzv = 23

    xmin = -2.0
    ymin = -3.0
    zmin = -4.0

    xmax =  2.0
    ymax =  3.0
    zmax =  4.0



def gauss(ux,uy,uz):

    #return ux + uy + uz

    delgam = np.sqrt(1.0)
    mux = 0.0
    muy = 0.0
    muz = 0.0

    #f  = 1.0/np.sqrt(2.0*np.pi*delgam)
    f = 1.0
    f *= np.exp(-0.5*((ux - mux)**2)/delgam)
    f *= np.exp(-0.5*((uy - muy)**2)/delgam)
    f *= np.exp(-0.5*((uz - muz)**2)/delgam)

    return f
    #return 1.0



def level_fill(m, rfl):

    nx, ny, nz = m.get_length(rfl)
    for i in range(nx):
        for j in range(ny):
            for k in range(nz):
                x,y,z = m.get_center([i,j,k], rfl)
                val = gauss(x,y,z)
                m[i,j,k, rfl] =  val



def plotSlices(axs, m, rfl):


    nx, ny, nz = m.get_length(rfl)

    xmid = nx/2
    ymid = ny/2
    zmid = nz/2

    #print("xmid: ", xmid)
    #print("ymid: ", ymid)
    #print("zmid: ", zmid)

    xx = np.zeros((nx))
    yy = np.zeros((ny))
    zz = np.zeros((nz))

    fx = np.zeros((nx))
    fy = np.zeros((ny))
    fz = np.zeros((nz))

    for i in range(nx):
        x,y,z = m.get_center([i,ymid,zmid], rfl)
        val   = m[i, ymid, zmid, rfl]

        xx[i] = x
        fx[i] = val

    for j in range(ny):
        x,y,z = m.get_center([xmid,j,zmid], rfl)
        val   = m[xmid, j, zmid, rfl]

        yy[j] = y
        fy[j] = val

    for k in range(nz):
        x,y,z = m.get_center([xmid,ymid,k], rfl)
        val   = m[xmid, ymid, k, rfl]

        zz[k] = z
        fz[k] = val

    cols = ["k", "b", "r", "g"]

    #axs[0].step(xx, fx, ".-", color=cols[rfl])

    axs[0].step(xx, fx, where="mid", color=cols[rfl])
    axs[1].step(yy, fy, where="mid", color=cols[rfl])
    axs[2].step(zz, fz, where="mid", color=cols[rfl])


    #print(np.diff(xx))
    #print(np.diff(yy))
    #print(np.diff(zz))



    
def saveVisz(lap, conf):
    slap = str(lap).rjust(4, '0')
    fname = conf.outdir + '/amr_{}.png'.format(slap)
    plt.savefig(fname)




if __name__ == "__main__":

    # set up the grid
    ################################################## 
    m = pdev.AdaptiveMesh3D()
    m.resize( [conf.Nxv,  conf.Nyv,  conf.Nzv ])
    m.set_min([conf.xmin, conf.ymin, conf.zmin])
    m.set_max([conf.xmax, conf.ymax, conf.zmax])


    indices = [ [0,0,0], [1,1,1], [1,2,3] ]

    for indx1 in indices:
        print(" ")
        print("testing with: ", indx1)
        rfl1  = 4
        cid   = m.get_cell(indx1, rfl1)
        print("indx: ", indx1, rfl1, " cid:", cid)

        rfl2  = m.get_refinement_level(cid)
        print(" ")
        indx2 = m.get_indices(cid)
        print("indx: ", indx2, rfl2, " cid:", cid)


    
    print("max. possible refinement:", m.get_maximum_possible_refinement_level())

    sys.exit()

    level_fill(m, 0)
    level_fill(m, 1)
    level_fill(m, 2)
    level_fill(m, 3)



    ################################################## 
    # set up plotting and figure
    plt.fig = plt.figure(1, figsize=(8,9))
    plt.rc('font', family='serif', size=12)
    plt.rc('xtick')
    plt.rc('ytick')
    
    gs = plt.GridSpec(3, 1)
    gs.update(hspace = 0.5)
    
    axs = []
    axs.append( plt.subplot(gs[0]) )
    axs.append( plt.subplot(gs[1]) )
    axs.append( plt.subplot(gs[2]) )

    plotSlices(axs, m, 0)
    plotSlices(axs, m, 1)
    plotSlices(axs, m, 3)


    saveVisz(0, conf)


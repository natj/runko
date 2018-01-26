import numpy as np

import sys, os
sys.path.append('../../python')        #plasma, plasmatools
sys.path.append('../../corgi/pycorgi') #corgi mesh infrastucture

import corgi
import pyplasmaDev as pdev
import pyplasma as plasma

np.random.seed(0)



def spatialLoc(node, Ncoords, Mcoords, conf):

    #node coordinates
    i, j    = Ncoords 
    Nx      = conf.Nx
    Ny      = conf.Ny

    #mesh coordinates
    l, m, n = Mcoords 
    NxMesh = conf.NxMesh
    NyMesh = conf.NyMesh
    NzMesh = conf.NzMesh

    #grid spacing
    xmin = node.getXmin()
    ymin = node.getYmin()

    dx = conf.dx
    dy = conf.dy
    dz = conf.dz


    #calculate coordinate extent
    x = xmin + i*NxMesh*dx + l*dx
    y = ymin + j*NyMesh*dy + m*dy
    z = 0.0                + n*dz

    return (x, y, z)



def createEmptyVelocityMesh(conf):
    vmesh = pdev.AdaptiveMesh3D()
    vmesh.resize( [conf.Nxv,  conf.Nyv,  conf.Nzv ])
    vmesh.set_min([conf.vxmin, conf.vymin, conf.vzmin])
    vmesh.set_max([conf.vxmax, conf.vymax, conf.vzmax])

    return vmesh


def fillMesh(vmesh, ffunc, xloc, ispcs, conf):

    # standard flat fill on 0th rfl level
    nx, ny, nz = vmesh.get_size(0)
    for r in range(nx):
        for s in range(ny):
            for t in range(nz):
                uloc = vmesh.get_center([r,s,t], 0)

                val = ffunc(xloc, uloc, ispcs, conf)
                vmesh[r,s,t, 0] =  val #ref lvl 0

    if conf.refinement_level < 1:
        return 


    ###################################################
    # adaptivity

    adapter = pdev.Adapter();

    sweep = 1
    while(True):
        #print("-------round {}-----------".format(sweep))
        adapter.check(m)
        adapter.refine(m)

        print("cells to refine: {}".format( len(adapter.cells_to_refine)))
        for cid in adapter.cells_created:
            rfl = m.get_refinement_level(cid)
            indx = m.get_indices(cid)
            uloc = m.get_center(indx, rfl)

            val = ffunc(xloc, uloc, ispcs, conf)
            m[indx[0], indx[1], indx[2], rfl] = val

        adapter.unrefine(m)
        print("cells to be removed: {}".format( len(adapter.cells_removed)))

        sweep += 1
        if sweep > conf.refinement_level: break

    if conf.clip:
        vmesh.clip_cells(conf.clipThreshold)

    return 



#inject plasma into cells
def inject(node, ffunc, conf):

    #loop over all *local* cells
    for i in range(node.getNx()):
        for j in range(node.getNy()):
            #if n.getMpiGrid(i,j) == n.rank:
            if True:

                #get cell & its content
                cid    = node.cellId(i,j)
                print(i,j,cid)
                c      = node.getCellPtr(cid) #get cell ptr

                print(type(c))
                #blocks = c.getPlasma(0)       # timestep 0
                #print(type(blocks))

                # loop over species
                species = []
                for ispcs in range(2):
                    #block = c.getPlasmaSpecies(0, ispcs) #zeroth step & zeroth species

                    block = pdev.PlasmaBlock(conf.NxMesh, conf.NyMesh, conf.NzMesh)
                    block.qm = conf.qm #set q/m
                    print(type(block))

                    for n in range(conf.NzMesh):
                        for m in range(conf.NyMesh):
                            for l in range(conf.NxMesh):
                                xloc = spatialLoc(node, (i,j), (l,m,n), conf)

                                vmesh = createEmptyVelocityMesh(conf)
                                fillMesh(vmesh,
                                         ffunc,
                                         xloc,
                                         ispcs,
                                         conf)

                                block[l,m,n] = vmesh
                                #c.getPlasmaSpecies(0,ispcs,(l,m,n)) = vmesh

                    species.append(block)

                    vmesh  = block[0,0,0]
                    print("0:",len(vmesh.get_cells(True)))

                    c.insertInitialSpecies(species)


                block = c.getPlasmaSpecies(0,0)
                vmesh = block[0,0,0]
                print("1:",len(vmesh.get_cells(True)))

    cid    = node.cellId(0,0)
    c      = node.getCellPtr(cid) #get cell ptr
    block  = c.getPlasmaSpecies(0,0)       # timestep 0
    vmesh  = block[0,0,0]
    print("2:",len(vmesh.get_cells(True)))




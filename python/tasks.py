import corgi
import plasmatools as ptools
import pyplasma as plasma




# setup vlasov solvers for each thread
def init_solvers(node):

    #setup momentum space solver
    global vsol
    global intp

    vsol = plasma.MomentumLagrangianSolver()
    intp = ptools.BundleInterpolator4th()
    vsol.setInterpolator(intp)


    #setup spatial space solver
    global ssol

    ssol = plasma.SpatialLagrangianSolver2nd()
    ssol.setGrid(node)




# Make the momentum space update
def push_momentum(args):
    indx, node = args

    cell = node.getCellPtr( indx )
    vsol.setCell(cell)
    vsol.solve()

    return


# Make the spatial space update
def push_spatial(args):
    indx, node = args

    c = node.getCellPtr( indx )
    i = c.i
    j = c.j

    ssol.setTargetCell(i,j)
    ssol.solve()



def deposit_current(args):
    indx, node = args

    cell = node.getCellPtr( indx )
    cell.depositCurrent()

    return


def updateBoundaries(cid):
    c = node.getCellPtr( cid )
    c.updateBoundaries(node)


# Cycle all Vlasov containers ahead
def cycle(node):
    for cid in node.getCellIds():
        cell = node.getCellPtr( cid )
        cell.cyclePlasma()


# Cycle all Vlasov containers ahead
def clip(node):
    for cid in node.getCellIds():
        cell = node.getCellPtr( cid )
        cell.clip()




#!/usr/bin/env python
# =============================================================================
# $Id: HelloWorld.py,v 1.3 2004-11-08 17:02:46 ibelyaev Exp $
# =============================================================================
# CVS tag $Name: not supported by cvs2svn $ 
# =============================================================================
"""
Standard 'Hello, world!' example. No way to avoid it!
"""
# =============================================================================
# @file
#
# "Solution"-file for 'Hello,World!' example (Bender Tutorial)
#
# @author Vanya BELYAEV  belyaev@lapp.in2p3.fr
# @date   2004-10-12
# =============================================================================

# import everything from BENDER
from bendermodule import *

# =============================================================================
# @class HelloWorld
# simple Python/Bender class for classical 'Hello,World!' example
# =============================================================================
class HelloWorld(Algo):
    def analyse( self ) :
        print 'Hello, world!'
        return SUCCESS
# =============================================================================

# =============================================================================
# The configuration of the job
# =============================================================================
def configure() :

    gaudi.config( files = ['$BENDERTUTOROPTS/BenderTutor.opts' ] )

    # modify/update the configuration:
    
    # (1) create the algorithm
    alg = HelloWorld( 'Hello' )
    
    # (2) replace the list of top level algorithm by
    #     new list, which contains only *THIS* algorithm
    gaudi.setAlgorithms( [ alg ] )
    
    # define input data files :
    #    1) get the Event Selector from Gaudi
    #    2) configure Event Selector 
    evtSel = gaudi.evtSel()
    evtSel.open( [ 'LFN:/lhcb/production/DC04/v1/DST/00000543_00000017_5.dst' , 
                   'LFN:/lhcb/production/DC04/v1/DST/00000543_00000018_5.dst' ,
                   'LFN:/lhcb/production/DC04/v1/DST/00000543_00000016_5.dst' ,
                   'LFN:/lhcb/production/DC04/v1/DST/00000543_00000020_5.dst' ] )

    
    return SUCCESS 
# =============================================================================

# =============================================================================
# Job steering 
# =============================================================================
if __name__ == '__main__' :

    # job configuration
    configure()

    # event loop 
    gaudi.run(10)
    print ' 10 events have been processed '
    gaudi.run(5)
    print '  5 events have been processed '
    gaudi.run(1)
    print '  1  event has  been processed '
        
    # for the interactive mode it is better to comment the last line
    gaudi.exit()
# =============================================================================

# =============================================================================
# $Log: not supported by cvs2svn $
# =============================================================================
# The END
# =============================================================================



#!/usr/bin/env python 
# =============================================================================
# $Id$ 
# =============================================================================
## @file Bender/DstExplorer.py
#
#  Trivial Bender-based script to explore the content of (x,mu,s,r,...)DSTs
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  <b>``C++ ToolKit for Smart and Friendly Physics Analysis''</b>
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software''
#
# Usage:
#
# @code
#
#    dst_explorer [options] file1 [ file2 [ file3 [ file4 ....'
#
#    1) exploring (r,s,m,fm)DSTs:
#
#       >>> ls('/Event')
#       >>> ls('/Event/Charm')
#       >>> ls('/Event/Charm/Phys')
#       >>> ls('/Event/Charm/Phys/D2KKK')
#
#    2) getting particles from TES and loop over them :
# 
#       >>> Ds = get('/Event/Charm/Phys/D2KKK')
#       >>> for D in Ds : print D.decay()
#
#    3) go to the next event:
#
#       >>> run(1)
#       >>> ls('/Event')
#
#    4) loop over events in search for ``valid'' data
#
#       >>> Ds, evt = seekForData('/Event/Charm/Phys/DstarForPromptCharm/Particles' )
#
#    5) loop in search for event with certain Stripping Decision:
#
#       >>> ok, evt = seekStripDecision('.*Lam.*' )
# 
# 
#  @endcode 
#
#  @date   2010-09-10
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
# =============================================================================
"""
Trivial Bender-based script to explore the content of (x,mu,s,r,...)DSTs

This file is a part of BENDER project:

  ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
 
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement with the smear campaign of Dr.O.Callot et al.:

   ``No Vanya's lines are allowed in LHCb/Gaudi software''

Usage:

    dst_explorer [options] file1 [ file2 [ file3 [ file4 ....'

    1) exploring (r,s,m,fm)DSTs:

       >>> ls('/Event')
       >>> ls('/Event/Charm')
       >>> ls('/Event/Charm/Phys')
       >>> ls('/Event/Charm/Phys/D2KKK')

    2) getting particles from TES and loop over them :
 
       >>> Ds = get('/Event/Charm/Phys/D2KKK')
       >>> for D in Ds : print D.decay()

    3) go to the next event:

       >>> run(1)
       >>> ls('/Event')

    4) loop over events in search for ``valid'' data

       >>> Ds, evt = seekForData('/Event/Charm/Phys/DstarForPromptCharm/Particles' )

    5) loop in search for event with certain Stripping Decision:

       >>> ok, evt = seekStripDecision('.*Lam.*' )
 

                  $Revision$
Last modification $Date$
               by $Author$

"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@nikhef.nl'
__date__    = "2010-09-10"
__version__ = '$Revision$'
__all__     = ()  ## nothing to import 
__usage__   = 'dst_explorer [options] file1 [ file2 [ file3 [ file4 ....'
# =============================================================================
## logging
# =============================================================================
from Bender.Logger import getLogger 
logger = getLogger( __name__ )
# =============================================================================
## create the parser
def makeParser ( usage = None ,
                 vers  = None ) :
    """
    Create the parser 
    """
    #
    if not usage : usage = __usage__
    if not vers  : vers  = __version__
    #
    from optparse import OptionParser as OptParser
    parser = OptParser( usage   = usage            ,
                        version = ' %prog ' + vers )
    
    ##
    parser.add_option (
        '-t'                  ,
        '--datatype'          ,
        dest    = 'DataType'  ,
        type    = 'str'       , 
        help    = "``DataType''    attribute to be specified for DaVinci [default : %default]" ,
        default = '2011'  
        )
    ## 
    parser.add_option (
        '-s'                          ,
        '--simulation'                ,
        action  = "store_true"        ,
        dest    = 'Simulation'        ,
        help    = "``Simulation''  flag to be propagated to DaVinci" ,
        default = False   
        )
    ## 
    parser.add_option (
        '-c'                          ,
        '--castor'                    ,
        action  = "store_true"        ,
        dest    = 'Castor'            ,
        help    = "Enable direct access to Castor Grid Storage to access LFN-files" ,
        default = True   
        )
    ## 
    parser.add_option (
        '-l'                          ,
        '--level'                     ,
        type    = 'int'               , 
        dest    = 'OutputLevel'       ,
        help    = "``OutputLevel'' attribute for ApplicationMgr/MessageSvc [default : %default]" ,
        default = 3                  
        )
    ## 
    parser.add_option (
        '-m'                          ,
        '--micro'                     ,
        action  = "store_true"        ,
        dest    = 'MicroDST'          ,
        help    = "MicroDST format?"  ,
        default = False   
        )
    ##
    parser.add_option (
        '-x'                          ,
        '--xml'                        ,
        dest    = 'XmlCatalogue'       ,
        help    = "``XmlCatalog'' to be transferred to setData-function [default : %default]" ,
        default = ''                  
        )
    ## 
    parser.add_option (
        '-q'                          ,
        '--quiet'                     ,
        action  = "store_true"        ,
        dest    = 'Quiet'             ,
        help    = "``Quiet'' processing"  ,
        default = False   
        )
    ## 
    parser.add_option (
        '-r'                     ,
        '--root'                 ,
        type    = 'str'          ,
        dest    = 'RootInTES'    ,
        help    = 'Root-In-TES'  ,
        default = ''           
        )
    ##
    parser.add_option (
        '-u'                       ,
        '--useoracle'              ,
        action  = "store_true"     ,
        dest    = 'UseOracle'      ,
        help    = "Use Oracle-DB"  ,
        default = False   
        )
    ##
    return parser

# =============================================================================
## configure the application form parser data  
def configure ( options , arguments ) :
    """
    Configure the application from parser data 
    """
    #
    ## redefine output level for 'quiet'-mode
    if options.OutputLevel > 5 :
        options.OutputLevel = 5
        logger.info('set OutputLevel to be %s ' % options.OutputLevel )
        
    if options.OutputLevel < 0 :
        options.OutputLevel = 0
        logger.info('set OutputLevel to be %s ' % options.OutputLevel )
        
    if options.Quiet and 4 > options.OutputLevel :
        options.OutputLevel = 4
        logger.info('set OutputLevel to be %s ' % options.OutputLevel )
        
    if options.Simulation and '2009' == options.DataType :
        options.DataType = 'MC09'
        logger.info('set DataType to be MC09')
        
    if options.Simulation and '2010' == options.DataType :
        options.DataType = 'MC10'
        logger.info('set DataType to be MC10')
            
    #
    ## start the actual action:
    #
    from Configurables       import DaVinci    
    ext = "dst"
    for a in arguments :
        p   = a.rfind ( '.' )
        if 0 <= p : 
            ext = a[p+1:]
            break
        
    daVinci = DaVinci (
        DataType    = options.DataType    ,
        Simulation  = options.Simulation  ,
        Lumi        = False              
        )
    
    if options.MicroDST or 'mdst' == ext or 'MDST' == ext or 'uDST' == ext :
        daVinci.InputType = 'MDST'
        
    if options.RootInTES and '/' == options.RootInTES[-1] :
        options.RootInTES = options.RootInTES[:-1]
    if options.RootInTES and  0  != options.RootInTES.find ( '/Event/' ) :
        options.RootInTES = '/Event/' + options.RootInTES
        
    if options.RootInTES :
        from Bender.MicroDST import uDstConf 
        uDstConf(options.RootInTES)
        logger.info('Reconfigure uDST')
   
    if not options.Simulation and options.DataType in ( '2010' , '2011' ) :
        #
        ## try to use the latest available tags:
        #
        from Configurables import CondDB    
        CondDB ( UseLatestTags = [ options.DataType ] )
        import os 
        if options.UseOracle and os.environ.has_key('LHCBGRIDSYSROOT') :
            if os.environ.has_key('LHCBGRIDCONFIGROOT') :
                if os.path.exists ( os.environ['LHCBGRIDSYSROOT'] )  :
                    if os.path.exists ( os.environ['LHCBGRIDCONFIGROOT'] )  :
                        #
                        ## Use Oracle if possible
                        CondDB ( UseOracle = True  )
                        logger.info('Oracle DB will be used')
                        
                    
    ## Reset all DaVinci sequences 
    def _action ( ) :
        """
        Reset all DaVinci sequences
        """
        from Gaudi.Configuration import allConfigurables 
        from Gaudi.Configuration import getConfigurable 
        for seq in ( 'DaVinciInitSeq'      ,
                     'DaVinciMainSequence' ,
                     'DaVinciSequence'     ,
                     'MonitoringSequence'  ,
                     'FilteredEventSeq'    ) :

            if not seq in allConfigurables : continue 
            cSeq = getConfigurable( seq )
            if cSeq and hasattr ( cSeq , 'Members' ) : cSeq.Members = []

            ## reset the list of top-level algorithms 
            from Configurables import ApplicationMgr
            a = ApplicationMgr()
            a.TopAlg = []
            a.OutputLevel = options.OutputLevel
            
            from Configurables import MessageSvc
            m = MessageSvc ( OutputLevel = options.OutputLevel )
            
            from GaudiConf import IOHelper
            ioh = IOHelper () 
            ioh.setupServices()
            
    ## prepare to copy good/marked/tagged evenst
    if hasattr ( options, 'OutputFile' ) and options.OutputFile :
        from Bender.Utils import copyGoodEvents
        if 0 <= options.OutputFile.find ( '.' ) : 
            copyGoodEvents (             options.OutputFile         ) 
        else :
            copyGoodEvents ( "%s.%s" % ( options.OutputFile , ext ) ) 

    from Gaudi.Configuration import appendPostConfigAction
    appendPostConfigAction ( _action )
    
    ## get xml-catalogs (if specified) 
    catalogs = [ options.XmlCatalogue ] if options.XmlCatalogue else []
    
    ## set input data
    from Bender.Main import setData 
    setData ( arguments , catalogs , options.Castor  )

    if not options.Quiet : print daVinci
        
# =============================================================================
if '__main__' == __name__ :
    
    print 120*'*'
    
    print ' Author  : ', __author__ 
    print ' Version : ', __version__ 
    print ' Date    : ', __date__ 
    
    
    parser = makeParser  ( usage = __usage__   ,
                           vers  = __version__ )
    
    parser.add_option (
        '-o'                     ,
        '--output'               ,
        type    = 'str'          ,
        dest    = 'OutputFile'   ,
        help    = 'The name of output file with selected events' ,
        default = 'GoodEvents'           
        )
    ##
    options , arguments = parser.parse_args()
    
    print 120*'*'
    if options.Quiet :
        print ' Trivial Bender-based script to explore the content of (x,mu,s,r,...)DSTs '
    else :
        print __doc__

    
    ## Files must be specfied are mandatory!
    if not arguments : parser.error ( 'No input files are specified' ) 
    
    configure ( options , arguments ) 
    
    if options.Simulation : from Bender.MainMC   import *
    else                  : from Bender.Main     import *
    
    ## instantiate the application manager 
    gaudi  = appMgr ()
    
    evtSel = gaudi.evtSel() 

    ## initialize and read the first event
    run ( 1 )
    
# =============================================================================
# The END 
# =============================================================================


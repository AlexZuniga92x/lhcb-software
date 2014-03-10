#!/usr/bin/env python
# -*- coding: utf-8 -*-
# ==========================================================================================
# $Id:$
# ==========================================================================================
## @file  PyTMVA.py
#
#  Python interface to TMVA: Trainer and Reader 
#
#  Actually for Trainer, it is a bit simplified version of Albert's code 
#  @thanks Albert PUIG
#  Inspired from
#  @see http://www.slac.stanford.edu/grp/eg/minos/ROOTSYS/cvs/tmva/test/TMVAClassification.py
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly 
#   Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  ``C++ ToolKit for Smart and Friendly Physics Analysis''
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
#
#  @date   2013-10-02
#  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
#  @thanks Albert PUIG
#
#                    $Revision:$
#  Last modification $Date:$
#                 by $Author:$ 
# =============================================================================
"""
Python interface to two major TMVA classes :

-  Trainer
-  Reader 

Actually for Trainer, it is a bit simplified version of Albert's code [thanks Albert Puig],
inspired from
http://www.slac.stanford.edu/grp/eg/minos/ROOTSYS/cvs/tmva/test/TMVAClassification.py

This file is a part of BENDER project:
    ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
    ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement 
with the smear campain of Dr.O.Callot et al.: 
    ``No Vanya's lines are allowed in LHCb/Gaudi software.''

"""
# =============================================================================
__author__  = 'Vanya BELYAEV  Ivan.Belyaev@itep.ru'
__date__    = "2013-10-02"
__version__ = '$Revision:$'
__all__     = ( "Trainer" , "Reader" )
# =============================================================================
import ROOT
from   AnalysisPython.Logger  import getLogger
logger = getLogger( 'PyTMVA' )
# =============================================================================
## get the direct access to various TMVA macros&scripts 
try :
    ##
    import os 
    mpath  = ROOT.gEnv.GetValue("Unix.*.Root.MacroPath",'')
    mpath  = mpath.split(':')
    mpath += [ '$ROOTSYS/tmva/test' ]
    for i in range(0,len(mpath) ) :
        mpath[i] = os.path.expandvars ( mpath[i] )
        mpath[i] = os.path.expandvars ( mpath[i] )
        mpath[i] = os.path.expandvars ( mpath[i] )
        
    mpath = ':'.join( mpath )
    mpath = mpath.replace('::',':')
    ROOT.gEnv.SetValue("Unix.*.Root.MacroPath",mpath)
    ##
    logger.info    ( 'Update MacroPath to get the direct access to TMVA macros')
    ## 
except :
    logger.warning ( 'Unable to update MacroPath')
    pass 

    
# =============================================================================
## @class TMVATrainer
#  Helper class to train TMVA
#
#  @code
#
#  from PyTMVA import Trainer 
#  t = Trainer( methods =  [
#  ## type, name, configuration 
#  ( ROOT.TMVA.Types.kMLP ,
#    "MLP",
#    "H:!V:EstimatorType=CE:VarTransform=N:NCycles=600:HiddenLayers=N+7:TestRate=5:!UseRegulator"
#  ) ] )
#
#  varlist = [
#    'dtfchi2' , 
#    'ctau'    , 
#    'ptb'     , 
#    'vchi2'   
#  ]
#
#  t.train ( var_list                              ,
#            signal          = treeSignal          ,
#            background      = treeBackgrund       ,
#            outputfile      = 'output.root'       ,
#            signal_cuts     = cuts_for_signal     ,
#            background_cuts = cuts_for_background ,
#            spectators      = []                  ) 
#
#  @endcode 
#
#  For more detailes
#  @see http://www.slac.stanford.edu/grp/eg/minos/ROOTSYS/cvs/tmva/test/TMVAClassification.py
# 
#  @date   2013-10-02
#  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
#  @thanks Albert PUIG
class Trainer(object):
    """
    Helper class to train TMVA
    
    #
    #  from PyTMVA import Trainer 
    #  t = Trainer( methods =  [
    #  ## type, name, configuration 
    #  ( ROOT.TMVA.Types.kMLP ,
    #    'MLP',
    #    'H:!V:EstimatorType=CE:VarTransform=N:NCycles=600:HiddenLayers=N+7:TestRate=5:!UseRegulator'
    #  ) ] )
    #
    #  varlist = [
    #    'dtfchi2' , 
    #    'ctau'    , 
    #    'ptb'     , 
    #    'vchi2'   
    #  ]
    #
    #  t.train ( var_list                              ,
    #            signal          = treeSignal          ,
    #            background      = treeBackgrund       ,
    #            outputfile      = 'output.root'       ,
    #            signal_cuts     = cuts_for_signal     ,
    #            background_cuts = cuts_for_background ,
    #            spectators      = []                  ) 
    #
    
    """
    ## constructor
    #  @code
    # 
    #  from TMVA import Trainer 
    #  t = Trainer( methods = [
    #  ## type                   name   configuration 
    #  ( ROOT.TMVA.Types.kMLP , "MLP", "H:!V:EstimatorType=CE:VarTransform=N:NCycles=600:HiddenLayers=N+7:TestRate=5:!UseRegulator" ) 
    #  ] )
    #
    #  @endcode
    #  For more detailes
    #  @see http://www.slac.stanford.edu/grp/eg/minos/ROOTSYS/cvs/tmva/test/TMVAClassification.py.
    def __init__(self, methods ):
        """
        Constructor with list of mehtods
        >>> from PyTMVA import Trainer
        >>> methods = ....
        >>> trainer = Trainer ( methods )
        
        """
        self.methods  = methods
        self._verbose = False

    ## define the verbosity 
    def setVerbose(self, verbosity ): self._verbose =  bool( verbosity )

    # =========================================================================
    ## train TMVA 
    #  @code  
    #  varlist = [
    #    'dtfchi2'       , 
    #    'ctau'          ,
    #    'ptb'           , 
    #    'vchi2'         , 
    #  ]
    #
    #  trainer.train ( var_list                       ,
    #          signal          = treeSignal          ,
    #          background      = treeBackgrund       ,
    #          outputfile      = 'output.root'       ,
    #          signal_cuts     = cuts_for_signal     ,
    #          background_cuts = cuts_for_background ,
    #          spectators      = []                  ) 
    #  @endcode  
    # 
    def train ( self   , var_list    ,
                signal , background  ,
                outputfile           ,
                signal_cuts     = '' ,
                background_cuts = '' ,
                spectators      = [] ) :
        """
        Train the TMVA:
        >>> trainer.train ( var_list , ... ) 
        """
        #
        ## get the logger
        # 
        from   AnalysisPython.Logger  import getLogger
        ln = outputfile  
        p  = ln.find('.root')
        if 0 < p : ln = ln [ : p ]
        logger = getLogger ( "Trainer/%s" % ln ) 
        # 
        
        outFile = ROOT.TFile.Open   ( outputfile, 'RECREATE' )
        logger.info ( 'Output ROOT file: %s ' % outputfile )

        factory = ROOT.TMVA.Factory (
            "TMVAClassification"  ,
            outFile               ,
            "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" ) 
        logger.info ( 'Book TMVA-factory' ) 
        
        factory.SetVerbose(self._verbose)
        #
        for v in var_list :
            vv = v
            if isinstance ( vv , str ) : vv = ( vv , 'F' ) 
            factory.AddVariable  ( *vv )
            
        for v in spectators :
            vv = v
            if isinstance ( vv , str ) : vv = ( vv , 'F' )             
            factory.AddSpectator ( *vv )
        #
        signalWeight     = 1.0
        backgroundWeight = 1.0
        #
        logger.info ( ' Signal     cuts: "%s" ' %     signal_cuts )
        logger.info ( ' Background cuts: "%s" ' % background_cuts ) 
        # 
        factory.AddTree ( signal     , 'Signal'     ,     signalWeight ,
                          ROOT.TCut (      signal_cuts ) )
        factory.AddTree ( background , 'Background' , backgroundWeight ,
                          ROOT.TCut (  background_cuts ) )
        #
        factory.PrepareTrainingAndTestTree(
            ROOT.TCut ( signal_cuts     ) ,
            ROOT.TCut ( background_cuts ) ,
            "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" )
        #
        for m in self.methods :
            factory.BookMethod( *m )

        # Train MVAs
        logger.info  ( "Train    all Methods" )  
        factory.TrainAllMethods    ()
        # Test MVAs
        logger.info  ( "Test     all Methods" )  
        factory.TestAllMethods     ()
        # Evaluate MVAs
        logger.info  ( "Evaluate all Methods" )  
        factory.EvaluateAllMethods ()
        # Save the output.
        logger.info  ( "Output ROOT file is closed" )  
        outFile.Close()
        # Now move the weights files
        import glob, os, shutil 
        for weightFile in glob.glob("weights/TMVAClassification*.xml"):
            typeWeight = os.path.splitext(weightFile)[0].split("_")[-1].replace('.weights', '')
            outWeight  = os.path.splitext(outputfile)[0] + '_%s_weights.xml' % typeWeight
            shutil.move   ( weightFile, outWeight)
            logger.info   ( "Weights file is created: %s" % outWeight )  
            shutil.rmtree ( 'weights')

# =============================================================================
## @class Reader
#  Rather generic python interface to TMVA-reader
#  @attention It is not CPU-efficient
#  Ugly tricks with arrays are required to bypass some technical limitations
#
#  @code
#
#  r = Reader ( 'MyTMVA' ,
#       variables = [
#       ## name      accessor  
#       ( 'pt'   , lambda s : s.pt ) ,
#       ## name      accessor  
#       ( 'ip'   , lambda s : s.ip ) ,
#       ## name     
#         'var1'                     , ## use s.var1 
#       ## name     
#         'var2'                     , ## use s.var2 
#       ] ,
#       weights_file = 'my_weights.xml'
#      )
#  
#  @endcode
#  @see TMVA::Reader
#  @date   2013-10-02
#  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
#  @thanks Alexander BARANOV
class Reader(object)  :
    """
    Rather generic python interface to TMVA-reader
    
    #
    #  r = Reader ( 'MyTMVA' ,
    #       variables = [
    #       ## name      accessor  
    #       ( 'pt'   , lambda s : s.pt ) ,
    #       ## name      accessor  
    #       ( 'ip'   , lambda s : s.ip ) ,
    #       ## name     
    #         'var1'                     , ## use s.var1 
    #       ## name     
    #         'var2'                     , ## use s.var2 
    #       ] ,
    #       weights_file = 'my_weights.xml'
    #      )
    #  
    """
    def __init__ ( self         ,
                   name         , 
                   variables    ,
                   weights_file ) :
        
        self.reader = ROOT.TMVA.Reader()
        self.name   = name
        
        ## get the logger
        # 
        from   AnalysisPython.Logger  import getLogger
        self.logger = getLogger ( "Reader /%s" % name )

        from array import array

        self._variables = []
        
        for v in variables : 

            if   isinstance ( v , str ) :
                
                vname  = v
                fvun   = lambda s : getattr ( s , vname )
                vfield = array ( 'f' , [1] )
                
            elif isinstance ( v , tuple ) and 2 == len ( v ) :

                vname  = v[0]
                vfun   = v[1]
                vfield = array ( 'f' , [1] )

            else :
                
                self.logger.error    ('Invalid variable description!')
                raise AttributeError, 'Invalid variable description!'

            ##                     name    accessor   address 
            self._variables += [ ( vname , vfun     , vfield  ) ] 


        ## declare all varibales to TMVA.Reader 
        for v in self._variables :
            self.reader.AddVariable ( v[0] , v[2] )
            
            
        self.reader.BookMVA( self.name , weights_file )
        self.logger.info ('TMVA-reader is booked %s %s ' % ( self.name , weights_file ) ) 

    # =========================================================================
    ## evaluate TMVA
    #  @attention It is not CPU efficient
    #  Ugly trick with arrays is needed due to some technicla problems
    #  (actually TMVA reader needs the address of ``float''(in C++ sense) variable
    def __call__ ( self , entry ) :
        """
        Evaluate TMVA
        - It is not CPU efficient :-( 
        - Ugly trick with arrays is needed due to some pure technical problem
        [actually TMVA reader needs the address of ``float''(in C++ sense) variable]
        """
        
        ## loop over all variables 
        for v in self._variables :
            vfun    = v[1]           ## accessor function 
            v[2][0] = vfun ( entry ) ## fill variable from the tree/chain 
            
        ## evaluate TMVA 
        return self.reader.EvaluateMVA( self.name ) 


# =============================================================================
if '__main__' == __name__ :
    
    print '*'*120
    print                      __doc__
    print ' Author  : %s ' %   __author__    
    print ' Version : %s ' %   __version__
    print ' Date    : %s ' %   __date__
    print '*'*120
    
# =============================================================================
# The END 
# =============================================================================

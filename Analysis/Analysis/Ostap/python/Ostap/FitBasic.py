#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
## @file FitBasic.py
#
#  Set of useful basic utilities to build various fit models 
#
#  @author Vanya BELYAEV Ivan.Belyaeve@itep.ru
#  @date 2011-07-25
# 
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
# =============================================================================
"""Set of useful basic utilities to build various fit models"""
# =============================================================================
__version__ = "$Revision:"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2011-07-25"
__all__     = (
    ##
    'makeVar'     , ## helper function to create the proper RooRealVar
    'H1D_dset'    , ## convertor of 1D-histo to RooDataHist 
    'H2D_dset'    , ## convertor of 2D-histo to RooDataHist 
    'H1D_pdf'     , ## convertor of 1D-histo to RooHistPdf 
    'H2D_pdf'     , ## convertor of 1D-histo to RooDataPdf
    ##
    'PDF'         , ## useful base class for 1D-models
    'MASS'        , ## useful base class to create "signal" PDFs for mass-fits
    'Fit1D'       , ## the model for 1D-fit: signal + background + optional components  
    'Fit2D'       , ## the model for 2D-fit: signal + background + optional components
    ##
    'Adjust1D'    , ## addjust PDF to avoid zeroes (sometimes useful)
    'Convolution' , ## helper utility to build convolution 
    )
# =============================================================================
import ROOT, math
from   AnalysisPython.Logger     import getLogger
from   Ostap.PyRoUts             import VE, hID, allInts, cpp, rootID
from   Ostap.PyRoUts             import h1_axis , h2_axes 
from   Ostap.Utils               import RooSilent, NoContext 
# =============================================================================
if '__main__' ==  __name__ : logger = getLogger ( 'Ostap.FitBasic' )
else                       : logger = getLogger ( __name__         )
# =============================================================================
_nemax = 80000  ## number of events per CPU-core 
_ncmax =     6  ## maximal number of CPUs: there are some problems with >= 7
                ## @see https://sft.its.cern.ch/jira/browse/ROOT-4897
#
_ncpus = []
# 
def ncpu (  events ) :
    #
    #
    ### return  ROOT.RooFit.Save()
    #
    n  = events // _nemax
    if n       <= 1 : return ROOT.RooFit.Save() ## fake!!! 
    # 
    import multiprocessing
    n_cores = multiprocessing.cpu_count()
    if n_cores <= 1 : return ROOT.RooFit.Save () ## fake!!! 
    #
    num = min ( n , n_cores , _ncmax )
    if not _ncpus :
        _ncpus.append ( num )   
    #
    return ROOT.RooFit.NumCPU ( num )

# =============================================================================
## create/modify  the variable
#  Helper function for creation/modification/adjustment of variable
#  @code
#    v = makeVar ( 10   , 'myvar' , 'mycomment' )
#    v = makeVar ( 10   , 'myvar' , 'mycomment' , '' ,     -1 , 1 )
#    v = makeVar ( 10   , 'myvar' , 'mycomment' , '' , 0 , -1 , 1 )
#    v = makeVar ( None , 'myvar' , 'mycomment' , '' , 0 , -1 , 1 )
#    v = makeVar ( None , 'myvar' , 'mycomment' , 10 , 0 , -1 , 1 )
#    v = makeVar ( v    , 'myvar' , 'mycomment' , 10 )
#  @endcode
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2013-12-01
def makeVar ( var , name , comment , fix  , *args ) :
    """
    Make/modify  the variable:
    
    v = makeVar ( 10   , 'myvar' , 'mycomment' )
    v = makeVar ( 10   , 'myvar' , 'mycomment' , '' ,     -1 , 1 )
    v = makeVar ( 10   , 'myvar' , 'mycomment' , '' , 0 , -1 , 1 )
    v = makeVar ( None , 'myvar' , 'mycomment' , '' , 0 , -1 , 1 )
    v = makeVar ( None , 'myvar' , 'mycomment' , 10 , 0 , -1 , 1 )
    v = makeVar ( v    , 'myvar' , 'mycomment' , 10 )
    
    """
    #
    if   isinstance   ( var , tuple ) :
        var = ROOT.RooRealVar ( name , comment , var[0] , var[1] ) 
        
    if   isinstance   ( var , ( float , int , long ) ) :
        if   not    args  : var = ROOT.RooRealVar ( name , comment , var             )
        elif 2==len(args) : var = ROOT.RooRealVar ( name , comment , var , *args     )
        elif 3==len(args) : var = ROOT.RooRealVar ( name , comment , var , *args[1:] )
        
    ## create the variable from parameters 
    if not isinstance ( var , ROOT.RooAbsReal ) : 
        var = ROOT.RooRealVar ( name , comment , *args )
        
    ## fix it, if needed 
    if isinstance ( fix , ( float , int , long ) ) :
        
        if fix < var.getMin() :
            logger.warning("Min-value for %s is redefined to be %s " % ( var.GetName() , fix ) )
            var.setMin ( fix )
            
        if fix > var.getMax() :
            logger.warning("Max-value for %s is redefined to be %s " % ( var.GetName() , fix ) )
            var.setMax ( fix )
            
        if not var.isConstant () : var.fix    ( fix )
        else                     : var.setVal ( fix )

    return var

# =============================================================================
## helper class to temporary change a range for the variable 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class RangeVar(object) :
    """
    Helper class to temporary change a range for the variable 
    """
    def __init__( self , var , vmin , vmax ) :
        self.var  = var
        self.vmin = min ( vmin , vmax ) 
        self.vmax = max ( vmin , vmax )
        self.omin = self.var.getMin ()
        self.omax = self.var.getMax ()
        
    def __enter__ ( self ) :
        self.omin = self.var.getMin ()
        self.omax = self.var.getMax ()
        self.var.setMin ( self.vmin ) 
        self.var.setMax ( self.vmax )
        return self
    
    def __exit__  ( self , *_ ) :        
        self.var.setMin ( self.omin ) 
        self.var.setMax ( self.omax )


# =============================================================================
## @class PDF
#  The helper base class for implementation of 1D-pdfs 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2014-08-21
class PDF (object) :
    """
    Useful helper base class for implementation of generic PDFs for 1D-fit
    """
    def __init__ ( self , name ) :
        self.name         = name
        self._signals     = ROOT.RooArgSet ()
        self._backgrounds = ROOT.RooArgSet ()
        self._components  = ROOT.RooArgSet ()
        ## take care about sPlots 
        self._splots      = []
        
    ## get all declared components 
    def components  ( self ) : return self._components
    ## get all declared signals 
    def signals     ( self ) : return self._signals
    ## get all declared backgrounds 
    def backgrounds ( self ) : return self._backgrounds 

    ## adjust PDF with some small positive portion to avoid bad regions...
    def adjust         ( self , value = 1.e-5 ) :
        """
        Adjust PDF with some small positive portion to avodi bad regions
        """
        if not hasattr ( self , 'orig_pdf' ) :

            self.orig_pdf        = self.pdf
            self.p0_pdf   = ROOT.RooPolynomial ( 'p0' , 'poly0' , self.mass            )

            ## add as component: 
            if hasattr ( self , 'alist1' ) and hasattr ( self , 'alist2' ) :
                
                self.orig_pdf        = self.pdf
                self.adj_alist1 = ROOT.RooArgList () 
                self.adj_alist2 = ROOT.RooArgList ()
                for i in self.alist1 : self.adj_alist1.add  ( i )
                for i in self.alist2 : self.adj_alist2.add  ( i )
                
                self.p0_value = makeVar ( None , 'p0val', 'p0value' , value , 0 , 0 , 1000 )
            
                self.adj_alist1.add ( self.p0_pdf   )
                self.adj_alist2.add ( self.p0_value )
                
            else :

                self.adj_alist1 = ROOT.RooArgList ( self.p0_pdf , self.orig_pdf ) 
                self.p0_value   = makeVar ( None , 'p0_value', 'value(p0)' , value , 0 , 0 , 1 )
                self.adj_alist2 = ROOT.RooArgList ( self.p0_value )
                
            self.pdf       = ROOT.RooAddPdf  ( "adjust_"    + self.orig_pdf.GetName () ,  
                                               "Adjust(%s)" % self.orig_pdf.GetName () ,
                                               self.adj_alist1 ,
                                               self.adj_alist2 )
            
    ## make the actual fit (and optionally draw it!)
    def fitTo ( self , dataset , draw = False , nbins = 100 , silent = False , *args , **kwargs ) :
        """
        Perform the actual fit (and draw it)
        """

        #
        ## treat the arguments properly
        # 
        _args = []
        for a in args :
            if not isinstance ( a , ROOT.RooCmdArg ) :
                logger.warning ( 'PDF(%s).fitTo, unknown argument type %s, skip it ' % ( self.name , type ( a ) ) ) 
                continue
            _args.append ( a )
            
        for k,a in kwargs.iteritems() :
            print 'LOOP INSIDE!!', k, a 
            if isinstance ( a , ROOT.RooCmdArg ) :
                logger.debug   ( 'PDF(%s).fitTo, add keyword argument %s' % ( self.name , k ) )  
                _args.append ( a )
                continue
            elif k.upper() in ( 'WEIGHTED' , 'SUMW2' , 'SUMW2ERROR' )  and isinstance ( a , bool ) and dataset.isWeighted() :
                _args.append   (  ROOT.RooFit.SumW2Error( a ) )
                logger.debug   ( 'PDF(%s).fitTo, add keyword argument %s/%s' % ( self.name , k , a ) )                 
            else : 
                logger.warning ( 'PDF(%s).fitTo, unknown/illegal keyword argument type %s/%s, skip it ' % ( self.name , k , type ( a ) ) )
                continue            

        _args = tuple( _args ) 

        
        if silent : from Ostap.Utils import RooSilent as Context
        else      : from Ostap.Utils import NoContext as Context
        
        #
        ## define silent context
        #
        context = Context ()         
        with context :
            
            result =  self.pdf.fitTo ( dataset   ,
                                       ROOT.RooFit.Save (   ) ,
                                       ncpu ( len ( dataset ) ) ,
                                       *_args     )
            
            if hasattr ( self.pdf , 'setPars' ) : self.pdf.setPars() 

        st = result.status()
        if 0 != st and silent :
            logger.warning ( 'PDF(%s).fitTo: status is %s. Refit in non-silent regime ' % ( self.name , st  ) )    
            return self.fitTo ( dataset , draw , nbins , False , *args , **kwargs )
        
        if 0 != st   : logger.warning ( 'PDF(%s).fitTo: Fit status is %s ' % ( self.name , st   ) )    
        qual = result.covQual()
        if 3 != qual : logger.warning ( 'PDF(%s).fitTo: covQual    is %s ' % ( self.name , qual ) ) 

        #
        ## check the integrals (is possible)
        if hasattr ( self , 'alist2' ) :
            
            nsum = VE()            
            for i in self.alist2 :
                nsum += i.as_VE() 
                if i.getVal() > 0.95 * i.getMax() :
                    logger.warning ( 'PDF(%s).fitTo Variable %s == %s [close to maximum %s]'
                                     % ( self.name , i.GetName() , i.getVal () , i.getMax () ) )
                    
            if not dataset.isWeighted() : 
                nl = nsum.value() - 0.10 * nsum.error()
                nr = nsum.value() + 0.10 * nsum.error()
                if not nl <= len ( dataset ) <= nr :
                    logger.error ( 'PDF(%s).fitTo is problematic:  sum %s != %s '
                                   % ( self.name , nsum , len( dataset ) ) )  
                    
        if hasattr ( self.pdf , 'setPars' ) : self.pdf.setPars()
            
        for s in self.components () : 
            if hasattr ( s , 'setPars' ) : s.setPars()
        for s in self.backgrounds() :  
            if hasattr ( s , 'setPars' ) : s.setPars() 
        for s in self.signals    () : 
            if hasattr ( s , 'setPars' ) : s.setPars() 
                
        if not draw :
            return result, None 
        
        #
        ## again the context
        # 
        context = Context () 
        with context :

            frame = self.mass.frame( nbins )
            #
            dataset  .plotOn ( frame )
            #
            iB = 0 
            for i in self.backgrounds() :
                cmp = ROOT.RooArgSet( i ) 
                self.pdf .plotOn (
                    frame ,
                    ROOT.RooFit.Components ( cmp                    ) ,
                    ROOT.RooFit.LineWidth  ( 2                      ) ,
                    ROOT.RooFit.LineStyle  ( ROOT.kDashed           ) ,
                    ROOT.RooFit.LineColor  ( ROOT.kBlue     + iB    ) )
                iB += 1

            iS = 0 
            for i in self.signals  () :
                cmp = ROOT.RooArgSet( i )         
                self.pdf .plotOn (
                    frame ,
                    ROOT.RooFit.Components ( cmp                    ) , 
                    ROOT.RooFit.LineWidth  ( 2                      ) ,
                    ROOT.RooFit.LineStyle  ( ROOT.kDotted           ) ,
                    ROOT.RooFit.LineColor  ( ROOT.kMagenta   + iS   ) )
                iS += 1

            iC = 0 
            for i in self.components  () :
                cmp = ROOT.RooArgSet( i )         
                self.pdf .plotOn (
                    frame ,
                    ROOT.RooFit.Components ( cmp                    ) , 
                    ROOT.RooFit.LineWidth  ( 2                      ) ,
                    ROOT.RooFit.LineStyle  ( ROOT.kDashDotted       ) ,
                    ROOT.RooFit.LineColor  ( ROOT.kOrange     + iC  ) )
                iC+= 1
                
            #
            ## the total fit curve
            # 
            self.pdf .plotOn ( frame , ROOT.RooFit.LineColor  ( ROOT.kRed ) )
            
            
            frame.SetXTitle('')
            frame.SetYTitle('')
            frame.SetZTitle('')
            
            frame.Draw()
            
            return result, frame 

    ## fit the histogram (and draw it)
    def fitHisto ( self , histo , draw = False , silent = False , *args , **kwargs ) :
        """
        Fit the histogram (and draw it)
        """
        
        if silent : from Ostap.Utils import RooSilent as Context
        else      : from Ostap.Utils import NoContext as Context

        with RangeVar( self.mass , *(histo.xminmax()) ) : 
            
            ## convert it! 
            context = Context () 
            with context :
                hdset     = H1D_dset ( '',  histo , self.mass )
                self.hset = hdset.dset
                
            ## fit it!!
            return self.fitTo ( self.hset , draw , len ( histo ) , silent , *args , **kwargs )

    ## perform sPlot-analysis 
    def sPlot ( self , dataset ) : 
        """
        Make sPlot analysis

        """
        if not hasattr ( self , 'alist2' ) :
            logger.error ('PDF(%s) has not attribute "alist2", no sPlot is possible' % self.name ) 
            raise AttributeError('PDF(%s) his not equipped for sPlot'                % self.name )
        
        splot = ROOT.RooStats.SPlot ( rootID( "sPlot_" ) ,
                                      "sPlot"            ,
                                      dataset            ,
                                      self.pdf           ,
                                      self.alist2        )
        
        self._splots += [ splot ]
        
        return splot 
    

        
# =============================================================================
## helper base class for implementation  of various helper pdfs 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class MASS(PDF) :
    """
    helper base class for implementation of various pdfs 
    """
    def __init__ ( self            ,
                   name            ,
                   mn       = None ,
                   mx       = None , 
                   mass     = None ,
                   mean     = None ,
                   sigma    = None ) : 

        ## intialize the base 
        PDF.__init__ ( self , name )
        
        m_name  = "m_%s"     % name
        m_title = "mass(%s)" % name

        if isinstance ( mass , tuple ) :
            mn      = mass [0]
            mx      = mass [1]
            mass    = None
            
        if isinstance ( mass , ROOT.TH1   ) :
            mn,mx   = mass.xminmax()
            m_title = mass.GetTitle ()
            mass    = None
            
        if isinstance ( mass , ROOT.TAxis ) :
            mn      = mass.GetXmin()
            mx      = mass.GetXmax()
            mass    = None
            
        if mass is None :
            if not isinstance ( mn , ( float , int , long ) ) :
                raise AttributeError( "MASS(%s): invalid 'min'-parameter %s" % ( name , mn ) ) 
            if not isinstance ( mx , ( float , int , long ) ) :
                raise AttributeError( "MASS(%s): invalid 'max'-parameter %s" % ( name , mx ) )
            
        #
        ## adjust the mass and edges, create if needed
        #
        self.mass = makeVar ( mass   ,
                              m_name , m_title ,
                              mass   , 
                              min ( mn , mx ) , max( mn , mx ) )
        #
        self._mn = self.mass.getMin ()
        self._mx = self.mass.getMax ()
        #
        _dm      = self._mx - self._mn 
        
        #
        ## mean-value
        # 
        self.mean = makeVar ( mean              ,
                              "mean_%s"  % name ,
                              "mean(%s)" % name , mean ,  self._mn  , self._mx )
        #
        if self.mean.isConstant() :
            if not self._mn <= self.mean.getVal() <= self._mx :
                raise AttributeError ( 'MASS(%s): Fixed mass %s is not in mass-range (%s,%s)' % ( name , self.mean.getVal() , self._mn , self._mx ) )
        elif hasattr ( self.mean , 'setMin' ) and hasattr( self.mean , 'setMax' ) : 
            self.mean.setMin ( max ( self.mean.getMin () , self.mass.getMin() - 0.1 * _dm ) )
            self.mean.setMax ( min ( self.mean.getMax () , self.mass.getMax() + 0.1 * _dm ) )
            logger.debug ( 'MASS(%s) Mean range is redefined to be (%s,%s)' % ( name , self.mean.getMin() , self.mean.getMax() ) ) 
            
        #
        ## sigma
        #
        sigma_max  = 2.0 * _dm / math.sqrt ( 12 )
        self.sigma = makeVar ( sigma               ,
                               "sigma_%s"   % name ,
                               "#sigma(%s)" % name , sigma , 0.01 * sigma_max , 0 , sigma_max )
        

# =============================================================================
## simple convertor of 1D-histo to data set
#  @author Vanya Belyaev Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class H1D_dset(object) :
    """
    Simple convertor of 1D-histogram into data set
    """
    def __init__ ( self         ,
                   name         ,
                   histo        ,
                   mass  = None ) :
        #
        ## use mass-variable
        #
        if mass : self.mass = mass 
        else    : self.mass = makeVar ( mass , 'm_%s' % name , 'mass(%s)' % name , None , *(histo.xminmax()) )
        
        self.vlst  = ROOT.RooArgList    ( self.mass )
        self.vimp  = ROOT.RooFit.Import ( histo     )
        self.dset  = ROOT.RooDataHist   (
            rootID ( 'hds_' ) ,
            "Data set for histogram '%s'" % histo.GetTitle() ,
            self.vlst  ,
            self.vimp  )

# =============================================================================
## simple convertor of 1D-histogram into PDF
#  @author Vanya Belyaev Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class H1D_pdf(H1D_dset) :
    """
    Simple convertor of 1D-histogram into PDF 
    """
    def __init__ ( self         ,
                   name         ,
                   histo        ,
                   mass  = None ) :
        
        H1D_dset.__init__ ( self , name , histo , mass )
        
        self.vset  = ROOT.RooArgSet  ( self.mass )
        
        #
        ## finally create PDF :
        #
        self.pdf    = ROOT.RooHistPdf (
            'hpdf_%s'            % name ,
            'HistoPDF(%s/%s/%s)' % ( name , histo.GetName() , histo.GetTitle() ) , 
            self.vset  , 
            self.dset  ) 
        
# =============================================================================
## simple convertor of 2D-histo to data set
#  @author Vanya Belyaev Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class H2D_dset(object) :
    """
    Simple convertor of 2D-histogram into data set
    """
    def __init__ ( self         ,
                   name         ,
                   histo2       ,
                   mass  = None ,
                   mass2 = None ) :
        #
        ## use mass-variable
        #
        self.mass  = makeVar ( mass  , 'm_%s'  % name , 'mass (%s)' % name , None , *(histo2.xminmax()) )
        self.mass1 = self.mass 
        self.mass2 = makeVar ( mass2 , 'm2_%s' % name , 'mass2(%s)' % name , None , *(histo2.yminmax()) )
        
        self.vlst  = ROOT.RooArgList    ( self.mass1 , self.mass2 )
        self.vimp  = ROOT.RooFit.Import ( histo2    )
        self.dset  = ROOT.RooDataHist   (
            rootID ( 'hds_' ) ,
            "Data set for histogram '%s'" % histo2.GetTitle() ,
            self.vlst  ,
            self.vimp  )


# =============================================================================
## simple convertor of 2D-histogram into PDF
#  @author Vanya Belyaev Ivan.Belyaev@itep.ru
#  @date 2013-12-01
class H2D_pdf(H2D_dset) :
    """
    Simple convertor of 2D-histogram into PDF 
    """
    def __init__ ( self         ,
                   name         ,
                   histo2       ,
                   mass  = None , 
                   mass2 = None ) :
        
        H2D_dset.__init__ ( name , histo2 , mass , mass2 )
        
        self.vset  = ROOT.RooArgSet  ( self.mass , self.mass2 )
        
        #
        ## finally create PDF :
        #
        self.pdf    = ROOT.RooHistPdf (
            'hpdf_%s'            % name ,
            'HistoPDF(%s/%s/%s)' % ( name , histo2.GetName() , histo2.GetTitle() ) , 
            self.vset  , 
            self.dset  )

# =============================================================================
## @class Fit1D
#  The actual model for 1D-mass fits 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2011-08-02
class Fit1D (PDF) :
    """
    The actual model for generic 1D-fits 
    """
    def __init__ ( self                       , 
                   signal                     , ## the main signal 
                   background       = None    , ## the main background 
                   othersignals     = []      , ## additional signal         components
                   otherbackgrounds = []      , ## additional background     components
                   others           = []      , ## additional non-classified components 
                   suffix           = ''      ,
                   name             = ''      ,
                   bpower           = 0       ) :  

        #
        if not name and signal.name : name = signal.name
        #
        PDF.__init__ ( self , name + suffix )
        #
        self.suffix     = suffix 
        self.signal     =      signal 
        self.mass       = self.signal.mass
        #
        from Ostap.FitBkgModels import Bkg_pdf 
        #
        if background : self.background = background 
        else          : self.background = Bkg_pdf  ( 'Background' + suffix ,
                                                     self.mass , power = bpower )

        #
        self.s = makeVar ( None , "S"+suffix , "Signal"     + suffix , None , 0 , 1.e+6 )
        self.b = makeVar ( None , "B"+suffix , "Background" + suffix , None , 0 , 1.e+6 )
        
        self.S = self.s
        self.B = self.b
        
        self.S_name = self.s.GetName()
        self.B_name = self.b.GetName()

        #
        self.alist1 = ROOT.RooArgList (
            self.signal     .pdf ,
            self.background .pdf 
            )
        # 
        self.alist2 = ROOT.RooArgList (
            self.s  ,
            self.b
            )
        
        self.nums = [ self.s , self.b ]

        #
        ## update the lists of PDFs (for drawing)
        #
        self.signals     ().add ( self.signal     .pdf )
        self.backgrounds ().add ( self.background .pdf )

        self.more_signals       = othersignals
        self.more_backgrounds   = otherbackgrounds
        self.more_components    = others


        #
        ## treat additional signals
        #
        for c in self.more_signals : 
            
            i = len ( self.signals() )
            
            if   isinstance ( c ,  ROOT.RooAbsPdf ) :
                self.alist1    .add ( c     )
                self.signals() .add ( c     ) 
            elif hasattr    ( c ,'pdf' )            :
                self.alist1    .add ( c.pdf )
                self.signals() .add ( c.pdf ) 
            else :
                logger.warning('Fit1D(%s) Unknown signal component type %s, skip it!' % ( self.name , type(c) ) ) 
                continue 
            
            si = makeVar ( None                            ,
                           "S%s_%d"  % ( suffix , i ) ,
                           "S(%d)%s" % ( i , suffix ) , None  ,  10 , 0  ,  1.e+6 )            
            self.alist2.add  ( si )
            
            setattr ( self , si.GetName() , si )             
            self.nums.append ( si ) 

        #
        ## treat additional backgounds 
        #
        for c in self.more_backgrounds : 
            
            i = len ( self.backgrounds() )
            
            if   isinstance ( c ,  ROOT.RooAbsPdf ) :
                self.alist1         .add ( c     )
                self.backgrounds () .add ( c     ) 
            elif hasattr    ( c ,'pdf' )            :
                self.alist1         .add ( c.pdf )
                self.backgrounds () .add ( c.pdf ) 
            else :
                logger.warning('Fit1D(%s) Unknown background component type %s, skip it!' % ( self.name , type(c) ) ) 
                continue 
            
            si = makeVar ( None                       ,
                           "B%s_%d"  % ( suffix , i ) ,
                           "B(%d)%s" % ( i , suffix ) , None  ,  10 , 0  ,  1.e+6 )            
            self.alist2.add  ( si )
            
            setattr ( self , si.GetName() , si )             
            self.nums.append ( si ) 


        #
        ## treat additional components
        #
        for c in self.more_components : 
            
            i = len ( self.components() )
            
            if   isinstance ( c ,  ROOT.RooAbsPdf ) :
                self.alist1         .add ( c     )
                self.components () .add ( c     ) 
            elif hasattr    ( c ,'pdf' )            :
                self.alist1         .add ( c.pdf )
                self.components () .add ( c.pdf ) 
            else :
                logger.warning('Fit1D(%s) Unknown additional component type %s, skip it!' % ( self.name , type(c) ) ) 
                continue 
            
            si = makeVar ( None                       ,
                           "O%s_%d"  % ( suffix , i ) ,
                           "O(%d)%s" % ( i , suffix ) , None  ,  10 , 0  ,  1.e+6 )            
            self.alist2.add  ( si )
            
            setattr ( self , si.GetName() , si )             
            self.nums.append ( si ) 
            
            
        #
        ## final PDF
        # 
        self.pdf  = ROOT.RooAddPdf  ( "model_"    + suffix ,
                                      "model(%s)" % suffix ,
                                      self.alist1 ,
                                      self.alist2 )


# =============================================================================
## @class Fit2D
#  The actual model for 2D-fits 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2011-07-25
class Fit2D (object) :
    """
    The actual model for 2D-fits 
    """
    def __init__ ( self              ,
                   #
                   signal_1          , 
                   signal_2          ,
                   suffix = ''       ,
                   #
                   bkg1       = None ,
                   bkg2       = None ,
                   bkgA       = None ,
                   bkgB       = None ,
                   bkg2D      = None ,
                   #
                   power1     = 0    ,
                   power2     = 0    ,
                   powerA     = 0    ,
                   powerB     = 0    ,
                   #
                   ## main components:
                   ss         = None , ## signal    (1) * signal     (2)
                   sb         = None , ## signal    (1) * bakcground (2) 
                   bs         = None , ## background(1) * signal     (2)
                   bb         = None , ## background-2D 
                   ## additional components 
                   components = []   ) :

        self.suffix    = suffix 
        self.signal1   = signal_1
        self.signal2   = signal_2
        
        self.m1        = signal_1.mass
        self.m2        = signal_2.mass

        #
        from Ostap.FitBkgModels import Bkg_pdf 
        #
        
        #
        ## First component: Signal(1) and Signal(2)
        # 
        self.ss_pdf = ROOT.RooProdPdf ( "S1S2pdf" + suffix ,
                                        "Sig(1) x Sig(2)"  ,
                                        self.signal1.pdf   ,
                                        self.signal2.pdf   )
        
        if bkg1 : self.bkg1 = bkg1
        else    : self.bkg1 = Bkg_pdf ( 'Bkg(1)' + suffix , self.m1 , power1 )
        #
        ## Second component: Background(1) and Signal(2)
        # 
        self.bs_pdf = ROOT.RooProdPdf ( "B1S2pdf" + suffix  ,
                                        "Bkg(1) x Sig(2)"   ,
                                        self.bkg1.pdf       ,
                                        self.signal2.pdf    )
        
        if bkg2 : self.bkg2 = bkg2 
        else    : self.bkg2 = Bkg_pdf ( 'Bkg(2)' + suffix , self.m2 , power2 )
        #
        ## Third component:  Signal(1) and Background(2)
        # 
        self.sb_pdf = ROOT.RooProdPdf ( "S1B2pdf" + suffix  ,
                                        "Sig(1) x Bkg(2)"   ,
                                        self.signal1.pdf    ,
                                        self.bkg2.pdf       )
        
        ## 
        self._bkgs = ( bkg1 , bkg2 , bkgA , bkgB ) 
        #
        ## fourth component: Background(1) and Background(2) 
        #
        if bkg2D : self._bb2D  = bkg2D
        # 
        if   bkg2D and isinstance ( bkg2D , ROOT.RooAbsPdf ) : self.bb_pdf = bkg2D 
        elif bkg2D and hasattr    ( bkg2D , 'pdf'          ) : self.bb_pdf = bkg2D.pdf
        else     :
            
            if bkgA : self.bkgA = bkgA
            else    : self.bkgA = Bkg_pdf ( 'Bkg(A)' + suffix , self.m1 , powerA )
            if bkgB : self.bkgB = bkgB
            else    : self.bkgB = Bkg_pdf ( 'Bkg(B)' + suffix , self.m2 , powerB )
            
            self.bb_pdf = ROOT.RooProdPdf ( "B1B2pdf" + suffix ,
                                            "Bkg(A) x Bkg(B)"  ,
                                            self.bkgA.pdf      ,
                                            self.bkgB.pdf      )
            
        #
        ## coefficients
        #
        self.ss = makeVar ( ss   ,
                            "S1S2"          + suffix ,
                            "Sig(1)&Sig(2)" + suffix , None , 1000  , 0 ,  1.e+6 )
        self.sb = makeVar ( sb   ,
                            "S1B2"          + suffix ,
                            "Sig(1)&Bkg(2)" + suffix , None ,  100  , 0 ,  1.e+6 )
        self.bs = makeVar ( bs   ,
                            "B1S2"          + suffix ,
                            "Bkg(1)&Sig(2)" + suffix , None ,  100  , 0 ,  1.e+6 )
        self.bb = makeVar ( bb   ,
                            "B1B2"          + suffix ,
                            "Bkg(1)&Bkg(2)" + suffix , None ,   10  , 0 ,  1.e+6 )

        self.SS_name = self.ss.GetName()
        self.SB_name = self.sb.GetName()
        self.BS_name = self.bs.GetName()
        self.BB_name = self.bb.GetName()
        
        self.alist1 = ROOT.RooArgList ( self.ss_pdf , self.sb_pdf ,
            self.bs_pdf ,
            self.bb_pdf )
        self.alist2 = ROOT.RooArgList (
            self.ss ,
            self.sb ,
            self.bs ,
            self.bb )

        #
        ## treat additional components (if specified)
        # 
        self.components  = components
        self._cmps       = []
        icmp = 0 
        for cmp in self.components :

            icmp += 1
            
            if   isinstance ( cmp , ROOT.RooAbsPdf         ) : pass 
            elif hasattr    ( cmp , 'pdf'                  ) : cmp = cmp.pdf 
            elif isinstance ( cmp , ( float , int , long ) ) and not isinstance ( cmp , bool ) :
                px  = ROOT.RooPolynomial ( 'Px%d'    % icmp + suffix ,
                                           'Px(%d)'  % icmp + suffix , self.m1 ) 
                py  = ROOT.RooPolynomial ( 'Py%d'    % icmp + suffix ,
                                           'Py(%d)'  % icmp + suffix , self.m2 ) 
                cmp = ROOT.RooProdPdf    ( "Pxy%d"   % icmp + suffix ,
                                           "Pxy(%d)" % icmp + suffix , px , py )  
                self._cmps += [ px,py,cmp]
            else :
                logger.error( 'Unknown type of component %d %d ' % ( imcp , type(cmp) ) )

                
            nn = makeVar ( None ,
                           'Cmp%d'   % icmp + suffix ,
                           'Cmp(%d)' % icmp + suffix ,
                           None ,  100  ,  0 , 1.e+6 )  
            self._cmps += [ nn ]

            self.alist1.add ( cmp )
            self.alist2.add ( nn  )

        #
        ## build final PDF 
        # 
        self.pdf  = ROOT.RooAddPdf  ( "model2D"      + suffix ,
                                      "Model2D(%s)"  % suffix ,
                                      self.alist1 ,
                                      self.alist2 )
        
        self._splots = []

    ## fit 
    def fitTo ( self            ,
                dataset         ,
                draw   = False  ,
                xbins  = 50     ,
                ybins  = 50     ,
                silent = False  , *args ) :
        """
        Perform the fit
        """
        result = self.pdf.fitTo ( dataset                  , 
                                  ROOT.RooFit.Save ()      ,
                                  ncpu ( len ( dataset ) ) ,
                                  *args                    )

        st   = result.status()
        if 0 != st   : logger.warning('Fit2D.fitTo: fit status is %s' % st   )
        qual = result.covQual()
        if 3 != qual : logger.warning('Fit2D.fitTo: covQual    is %s' % qual )
        
        #
        ## keep dataset (for drawing)
        #
        self.dataset = dataset
        
        if hasattr ( self , 'alist2' ) :
            
            nsum = VE()            
            for i in self.alist2 :
                nsum += i.as_VE() 
                if i.getVal() > 0.9 * i.getMax() :
                    logger.warning ( 'Variable %s == %s [close to maximum %s]'
                                     % ( i.GetName() , i.getVal () , i.getMax () ) )
                    
            if not dataset.isWeighted() : 
                nl = nsum.value() - 0.05 * nsum.error()
                nr = nsum.value() + 0.05 * nsum.error()
                if not nl <= len ( dataset ) <= nr :
                    logger.error ( 'Fit is problematic:  sum %s != %s '
                                   % ( nsum , len( dataset ) ) )  
                    
        nbins = xbins 
        if not draw :
            return result,None
        
        return result, self.draw ( None , dataset , nbins , ybins , silent , *args ) 
    
    ## draw the projection over 1st variable 
    def draw1 ( self , dataset = None , nbins = 100 , silent = True , *args ) :
        """Draw the projection over 1st variable"""
        return self.draw ( self.m1 , dataset , nbins , 20    , silent , *args ) 
    ## draw the projection over 12nd variable 
    def draw2 ( self , dataset = None , nbins = 100 , silent = True , *args ) :
        """Draw the projection over 2nd variable"""
        return self.draw ( self.m2 , dataset , nbins   , 20 , silent , *args ) 

    ## make 1D-plot 
    def draw ( self           ,
               drawvar = None ,
               dataset = None ,
               nbins   = 100  ,
               ybins   =  20  ,
               silent  = True ,
               projvar = None ,  
               *args )  : 
        """
        Make 1D-plot:
        """
        
        context = NoContext () 
        if silent : context = RooSilent()
        
        if not dataset :
            if hasattr ( self , 'dataset' ) : dataset = self.dataset 
            
        with context :
                
            if not drawvar :
                
                _xbins = ROOT.RooFit.Binning ( nbins ) 
                _ybins = ROOT.RooFit.Binning ( ybins ) 
                _yvar  = ROOT.RooFit.YVar    ( self.m2 , _ybins )
                _clst  = ROOT.RooLinkedList  ()
                hdata  = self.pdf.createHistogram ( hID() , self.m1 , _xbins , _yvar )
                hpdf   = self.pdf.createHistogram ( hID() , self.m1 , _xbins , _yvar )
                hdata.SetTitle(';;;')
                hpdf .SetTitle(';;;')
                _lst   = ROOT.RooArgList ( self.m1 , self.m2 )  
                if dataset : dataset.fillHistogram( hdata , _lst ) 
                self.pdf.fillHistogram  ( hpdf , _lst )

                hdata.lego ()
                hpdf .Draw ( 'same surf')
                
                return hpdf , hdata 
            
            frame = drawvar.frame( nbins )
            
            if dataset : dataset  .plotOn ( frame , *args )

            ## if projvar :
                
            ##     projvar.setBins( 10 )
            ##     s = ROOT.RooArgSet ( projvar ) 
            ##     self.projdata = ROOT.RooDataHist ( 'proj'  , 'project' , s , dataset )
            ##     print 'PROJECTION DATA', self.projdata
            ##     _args = list  ( args )
            ##     _args.append  ( ROOT.RooFit.ProjWData( self.projdata ) )
            ##     args  = tuple ( _args ) 
            ##     print 'ARGS: ', args

            self.pdf .plotOn ( frame ,
                               ## ROOT.RooFit.ProjWData( self.projdata ) , 
                               ROOT.RooFit.Components ( self.sb_pdf.GetName() ) ,
                               ROOT.RooFit.LineStyle  ( ROOT.kDashed   ) ,
                               ROOT.RooFit.LineColor  ( ROOT.kGreen    ) , *args )
            
            self.pdf .plotOn ( frame ,
                               ## ROOT.RooFit.ProjWData( self.projdata ) , 
                               ROOT.RooFit.Components ( self.bs_pdf.GetName() ) ,
                               ROOT.RooFit.LineStyle  ( ROOT.kDotted   ) ,
                               ROOT.RooFit.LineColor  ( ROOT.kMagenta  ) , *args )
            
            self.pdf .plotOn ( frame ,
                               ## ROOT.RooFit.ProjWData( self.projdata ) , 
                               ROOT.RooFit.Components ( self.bb_pdf.GetName() ) ,          
                               ROOT.RooFit.LineWidth  ( 1              ) ,
                               ROOT.RooFit.LineColor  ( ROOT.kBlack    ) , *args )
            
            self.pdf .plotOn ( frame ,
                               ## ROOT.RooFit.ProjWData( self.projdata ) , 
                               ROOT.RooFit.Components ( self.ss_pdf.GetName() ) ,
                               ROOT.RooFit.LineWidth  ( 1              ) ,
                               ROOT.RooFit.LineColor  ( ROOT.kRed      ) , *args )
            
            self.pdf .plotOn ( frame ,
                               ## ROOT.RooFit.ProjWData( self.projdata ) , 
                               ROOT.RooFit.LineColor  ( ROOT.kRed      ) , *args )
            
            frame.SetXTitle ( '' )
            frame.SetYTitle ( '' )
            frame.SetZTitle ( '' )
            
            frame.Draw()
            
            return frame

    ## fit the histogram (and draw it)
    def fitHisto ( self , histo , draw = False , silent = False , *args ) :
        """
        Fit the histogram (and draw it)
        """
        context = NoContext () 
        if silent : context = RooSilent() 
        
        ## convert it! 
        with context : 
            self.hdset = H2D_dset ( '',  histo , self.m1 , self.m2  )
            self.hset  = self.hdset.dset
            
        ## fit it!!
        return self.fitTo ( self.hset      ,
                            draw           ,
                            histo.nbinsx() ,
                            histo.nbinsy() ,
                            silent         , *args ) 
    
    ## make splot-analysis 
    def sPlot ( self     ,
                dataset  ,
                *args    ) : 
        """
        make sPlot analysis 
        """
        splot = ROOT.RooStats.SPlot ( rootID ( "sPlot_" ) ,
                                      "sPlot"             ,
                                      dataset             ,
                                      self.pdf            ,
                                      self.alist2         , *args ) 
        
        self._splots += [ splot ]
        
        return splot 

# =============================================================================
## simple class to adjust certaint PDF to avoid zeroes 
class Adjust1D(object) :
    """
    Simple class to adjust certaint PDF to avoid zeroes 
    """
    ## constructor
    def __init__ ( self          ,
                   name          ,
                   mass          ,
                   pdf           ,
                   value = 1.e-3 ,
                   frac  = True  ) :
        
        self.mass    = mass
        self.old_pdf = pdf

        self.p0_pdf  = ROOT.RooPolynomial( 'p0_%s'     % name ,
                                           'poly0(%s)' % name , self.mass ) 
        
        self.num_f   = makeVar ( None , 'valueT_%s'    % name ,
                                 'value/true(%s)'      % name ,
                                 None ,
                                 0    , 1 )
        
        #self.num_v   = makeVar ( None , 'valF_%s'       % name ,
        #                         'value/fixtive(%s)'    % name ,
        #                         value ,
        #                         value , 0 , 10 )  
        
        #
        self.alist1 = ROOT.RooArgList (
            self.old_pdf ,
            self.p0_pdf    
            )
        
        self.alist2 = ROOT.RooArgList (
            self.num_f    ,
            # self.num_v
            # self.num_s  ,
            # self.num_v
            )
        #
        ## final PDF
        # 
        self.pdf  = ROOT.RooAddPdf  ( "adjust_"    + name ,
                                      "Adjust(%s)" % name ,
                                      self.alist1 ,
                                      self.alist2 )
        
        

# =============================================================================
## @class Convolution
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2014-07-13
class Convolution(object):
    """
    Helper class to make a convolution PDF 
    """
    def __init__ ( self           ,
                   name           ,
                   pdf            ,
                   mass           , 
                   convolution    ,
                   useFFT  = True ) :
        
        ## store "old" pdf 
        self.orig_pdf = pdf
        self.mass     = mass

        if   isinstance ( convolution ,   ROOT.RooAbsPdf       ) : self.convolution = convolution
        elif isinstance ( convolution , ( float , int , long ) ) and not isinstance ( convolution , bool ) :
            self.cnv_mean  = makeVar (
                0.0  ,
                'CnvMean'       + name ,
                'cnv_mean (%s)' % name , 
                0.0  , 0 ) 
            self.cnv_sigma = makeVar (
                None ,
                'CnvSigma'      + name ,
                'cnv_sigma(%s)' % name ,
                convolution ,
                convolution ,
                0.05 * convolution , 10 * convolution )
            self.cnv_gauss = ROOT.RooGaussian (
                'CnvGauss'     + name , 
                'CnvGauss(%s)' % name ,
                self.mass , self.cnv_mean , self.cnv_sigma )
            self.convolution = self.cnv_gauss
        else :
            raise AttributeError ( 'Unknown convolution type %s ' % convolution )

        #
        if useFFT :
            
            nb = 20000
            if hasattr ( self , 'cnv_sigma' ) :
                dm  = mass.getMax() - mass.getMin()
                dm /= self.cnv_sigma
                nb  = max ( nb , 10 * int (  dm ) )
                logger.debug('Convolution: choose #bins %d' % nb )
                
            self.mass.setBins ( 20000 , 'cache' ) 
            self.pdf = ROOT.RooFFTConvPdf ( 'FFT'     + name ,
                                            'FFT(%s)' % name ,
                                            self.mass , self.orig_pdf , self.convolution )
        else      :
            self.pdf = ROOT.RooNumConvPdf ( 'CNV'     + name ,
                                            'CNV(%s)' % name ,
                                            self.mass , self.orig_pdf , self.convolution )
            if isinstance ( self.convolution , ROOT.RooGaussian ) :
                if hasattr ( self , 'cnv_mean' ) and hasattr ( self , 'cnv_sigma' ) :
                    self.pdf.setConvolutonWindow( self.cnv_mean , self.cnv_sigma , 5 )
                    
# =============================================================================
if '__main__' == __name__ :
    
    from Ostap.Line import line 
    logger.info ( __file__ + '\n' + line  )
    logger.info ( 80*'*' )
    logger.info ( __doc__  )
    logger.info ( 80*'*' )
    logger.info ( ' Author  : %s' %         __author__    ) 
    logger.info ( ' Version : %s' %         __version__   ) 
    logger.info ( ' Date    : %s' %         __date__      )
    logger.info ( ' Symbols : %s' %  list ( __all__     ) )
    logger.info ( 80*'*' ) 

# =============================================================================
# The END 
# =============================================================================

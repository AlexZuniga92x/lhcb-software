#!/usr/bin/env python 
# -*- coding: utf-8 -*-
# =============================================================================
# $Id$ 
# =============================================================================
## @file BenderTools/Parser.py
#
#  Trivial Parser for Bender-based scripts
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
#
#  @date   2010-09-10
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
# =============================================================================
"""Trivial Parser for Bender-based scripts

oooooooooo.                              .o8                     
`888'   `Y8b                            \"888                     
 888     888  .ooooo.  ooo. .oo.    .oooo888   .ooooo.  oooo d8b 
 888oooo888' d88' `88b `888P\"Y88b  d88' `888  d88' `88b `888\"\"8P 
 888    `88b 888ooo888  888   888  888   888  888ooo888  888     
 888    .88P 888    .o  888   888  888   888  888    .o  888     
o888bood8P'  `Y8bod8P' o888o o888o `Y8bod88P\" `Y8bod8P' d888b    
                                                                 
This file is a part of BENDER project:

  ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
 
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement with
the smear campaign of Dr.O.Callot et al.:

   ``No Vanya's lines are allowed in LHCb/Gaudi software''


                  $Revision$
Last modification $Date$
               by $Author$

"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@nikhef.nl'
__date__    = "2010-09-10"
__version__ = '$Revision$'
__all__     = ( 'makeParser' ,
                'dataType'   ,
                'theYear'    ,
                'fileList'   )
# =============================================================================
## logging
# =============================================================================
from Bender.Logger import getLogger
if '__main__' == __name__ : logger = getLogger ( 'BenderTools.Parser' )
else                      : logger = getLogger ( __name__ )
# =============================================================================
import argparse
# =============================================================================
## @class Collect
#  simple parsing action to collect multiple arguments
#  @code
#  parser =...
#  parser.add_argument('--foo',
#  ... action  = Collect ,
#  ... nargs   = '*'     ,
#  ... default = []      ,
#  ...)
#  print parser.parse_args('a.txt b.txt --foo 1 2 3 --foo 4 -foo 5 '.split())
#  @encode
class Collect(argparse.Action):
    """Simple parsing action to collect multiple arguments
    >>> parser =...
    >>> parser.add_argument('--foo',
    ... action  = Collect ,
    ... nargs   = '*'     ,
    ... default = []      ,
    ...)
    >>> print parser.parse_args('a.txt b.txt --foo 1 2 3 --foo 4 -foo 5 '.split())
    """
    def __init__(self            ,
                 option_strings  ,
                 dest            ,
                 nargs    =None  ,
                 const    =None  ,
                 default  =None  , 
                 type     =None  ,
                 choices  =None  ,
                 required =False ,
                 help     =None  ,
                 metavar  =None  ) :
        if nargs == 0:
            raise ValueError('nargs for Collect actions must be > 0; if arg '
                             'strings are not supplying the value to append, '
                             'the append const action may be more appropriate')
        if const is not None and nargs != argparse.OPTIONAL:
            raise ValueError('nargs must be %r to supply const' % argparse.OPTIONAL)
        super(Collect, self).__init__(
            option_strings = option_strings ,
            dest           = dest           ,
            nargs          = nargs          ,
            const          = const          ,
            default        = default        ,
            type           = type           ,
            choices        = choices        ,
            required       = required       ,
            help           = help           ,
            metavar        = metavar        )
        
    def __call__(self, parser, namespace, values, option_string=None):
        items = argparse._copy.copy(argparse._ensure_value(namespace, self.dest, []))
        ## the only one important line: 
        for v in values : items.append(v)
        setattr(namespace, self.dest, items)

# =============================================================================
## create the base parser, suitable for many bender-based scripts
#  @date   2010-09-10
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
def makeParser ( **kwargs ) :
    """
    Create the parser 
    """
   
    ## version:
    version = kwargs.pop('version', None )
    ## create the parser 
    parser = argparse.ArgumentParser( **kwargs )
    if version: 
        parser.add_argument ( '-v', '--version' , action='version', version = version )
    
    ##
    parser.add_argument (
        '-q'                          ,
        '--quiet'                     ,
        action  = "store_true"        ,
        dest    = 'Quiet'             ,
        help    = "``Quiet'' processing [defaut : %(default)s ]"  ,
        default = False   
        )
    ## 
    parser.add_argument (
        '-i'                       ,
        '--import'                 ,
        dest    = 'ImportOptions'  ,
        metavar = 'IMPORT'         ,
        action  = Collect          , 
        nargs   = '*'              ,
        default = []               , 
        help    = """List of files to be used for 'importOptions',
        e.g. input data [default:%(default)s].
        The files are imported at the end of configuration step"""
        )
    ##
    group_dv = parser.add_argument_group(
        'LHCb/DaVinci' ,
        'General LHCb/DaVinci application configuration')
    group_dv.add_argument (
        '-p'                          ,
        '--print'                     ,
        type     = int                , 
        choices  = range(1,7)         , 
        dest     = 'OutputLevel'      ,
        help     = "``OutputLevel'' attribute for ApplicationMgr/MessageSvc [default : %(default)s]"  ,
        default  = 3                  
        )
    ##
    group_dv.add_argument (
        '-d'                      ,
        '--datatype'              ,
        dest    = 'DataType'      ,
        type    =  str            , 
        help    = """``DataType''    attribute for DaVinci [default : %(default)s].
        Often it can be deduced from input file name/extension""" ,
        default = '2012'  
        )
    ## 
    group_dv.add_argument (
        '-s'                      ,
        '--simulation'            ,
        action  = "store_true"    ,
        dest    = 'Simulation'    ,
        help    = """``Simulation''  attribute for DaVinci [default: %(default)s].
        Often it can be deduced from input file name/extension""" ,
        default = False   
        )
    ##
    group_dv.add_argument (
        '-m'                          ,
        '--micro'                     ,
        action  = "store_true"        ,
        dest    = 'MicroDST'          ,
        help    = """``MicroDST''    attribute for DaVinci [default : %(default)s].
        Often it can be deduced from input file name/extension""" ,
        default = False   
        )
    ##
    group_dv.add_argument (
        '-r'                         ,
        '--root'                     ,
        type    = str                ,
        default = ''                 ,
        dest    = 'RootInTES'        ,
        help    = """Root-In-TES fpr uDST [default : %(default)s].
        Often it can be deduced from input file name/extension""" 
        )
    ##
    group_dv.add_argument (
        '-l'                       ,
        '--lumi'                   ,
        action  = "store_true"     ,
        dest    = 'Lumi'           ,
        help    = "``Lumi''       attribute for DaVinci [default : %(default)s]",
        default = False   
        )
    
    ##
    group_dv.add_argument (
        '-a'                       ,
        '--all'                    ,
        action  = "store_true"     ,
        dest    = 'IgnoreDQFlags'  ,
        help    = "Use ``ALL'', IgnoreDQFlags attribute for DaVinci [default : %(default)s]",
        default = False   
        )
    ##
    group_da = parser.add_argument_group(
        'Input data' ,
        'Properties related to input data and data access')
    ##
    group_da.add_argument (
        '-g'                       ,
        '--grid'                   ,
        nargs   = '*'              ,
        dest    = 'Grid'           ,
        help    = "Grid-site(s) to access LFN-files (has precedence over 'castor/eos', but grid proxy is needed) [default : %(default)s]" ,
        default = ['CERN']
        )
    ##
    group_da.add_argument (
        '--no-castor'              ,
        action  = 'store_false'    , 
        dest    = 'Castor'         ,
        help    = "Disable direct access to Castor/EOS storage for LFNs",
        default = True             ## use castor on default 
        )
    ## 
    group_da.add_argument (
        '-x'                        ,
        '--xml'                     ,
        dest    = 'XmlCatalogs'     ,
        help    = "``XmlCatalogs'' to be transferred to setData-function [default: %(default)s]" ,
        ## action  = 'append'       ,
        action  = Collect           ,
        nargs   = '*'               , 
        default = []                
        )
    ## 
    group_da.add_argument (
        '-k'                       ,
        '--klist'                  ,
        dest    = 'FileList'       ,
        nargs   = '*'              ,
        default = []               , 
        help    = "Files with the plain list of input file names [default: %(default)s]"
        )
    ##
    group_da.add_argument (
        "files" ,
        metavar = "FILE"          ,
        nargs   = '*'             ,
        default = []              , 
        help    = "Input data(&python) files to be processed [default: %(default)s]" )
    ##
    return parser

# =============================================================================
## try to extract the data type, simulation flag and file extension (type)
#
#  @code
#
#    >>> file_names  = ...
#    >>> data_type, simulation,ext = dataType ( file_names )
#
#  @endcode
#
def dataType ( files ) :
    """
    extract the data type,
    simulation type and input type from file name
    
    >>> file_names  = ...
    >>> data_type, simulation,ext = daataType ( file_names )
    
    """
    #
    if isinstance ( files  , str ) : files = [ files ]
    #
    dtype = ''
    simu  = False
    ext   = '' 
    for f in files  :
        #
        fu = f.upper() 
        if   0 <= fu.find ( 'COLLISION09'   ) : dtype = '2009'
        elif 0 <= fu.find ( 'COLLISION10'   ) : dtype = '2010'
        elif 0 <= fu.find ( 'COLLISION11'   ) : dtype = '2011'
        elif 0 <= fu.find ( 'COLLISION12'   ) : dtype = '2012'
        elif 0 <= fu.find ( 'COLLISION13'   ) : dtype = '2012' ## ATTENTION
        elif 0 <= fu.find ( 'COLLISION15'   ) : dtype = '2015'
        elif 0 <= fu.find ( 'COLLISION16'   ) : dtype = '2016'
        #
        elif 0 <= fu.find ( 'STRIPPING13'   ) : dtype = '2011'
        elif 0 <= fu.find ( 'STRIPPING17'   ) : dtype = '2011'
        elif 0 <= fu.find ( 'STRIPPING15'   ) : dtype = '2011'
        elif 0 <= fu.find ( 'STRIPPING19'   ) : dtype = '2012'
        #
        elif 0 <= fu.find ( 'STRIPPING20R1' ) : dtype = '2011'
        elif 0 <= fu.find ( 'STRIPPING20'   ) : dtype = '2012'
        #
        elif 0 <= fu.find ( 'STRIPPING21R1' ) : dtype = '2011'
        elif 0 <= fu.find ( 'STRIPPING21'   ) : dtype = '2012'
        #
        elif 0 <= fu.find ( 'STRIPPING22'   ) : dtype = '2015'
        elif 0 <= fu.find ( 'STRIPPING23'   ) : dtype = '2015'
        elif 0 <= fu.find ( 'STRIPPING24'   ) : dtype = '2015'
        #
        elif 0 <= fu.find ( '2K+10'         ) : dtype = '2010'
        elif 0 <= fu.find ( '2K+11'         ) : dtype = '2011'
        elif 0 <= fu.find ( '2K+12'         ) : dtype = '2012'
        elif 0 <= fu.find ( '2K+13'         ) : dtype = '2013'
        elif 0 <= fu.find ( '2K+15'         ) : dtype = '2015'
        elif 0 <= fu.find ( '2K+16'         ) : dtype = '2016'
        
        #
        if   0 <= fu.find ( 'MC09' ) or 0 <= fu.find ( 'MC/2009' ) :
            dtype = '2009'
            simu  = True 
        elif 0 <= fu.find ( 'MC10' ) or 0 <= fu.find ( 'MC/2010' ) :
            dtype = '2010'
            simu  = True 
        elif 0 <= fu.find ( 'MC11' ) or 0 <= fu.find ( 'MC/2011' ) :  
            dtype = '2011'
            simu  = True 
        elif 0 <= fu.find ( 'MC12' ) or 0 <= fu.find ( 'MC/2012' ) :
            dtype = '2012'
            simu  = True
        elif 0 <= fu.find ( 'MC13' ) or 0 <= fu.find ( 'MC/2013' ) :
            dtype = '2013' ## ?? 
            simu  = True
        elif 0 <= fu.find ( 'MC15' ) or 0 <= fu.find ( 'MC/2015' ) :
            dtype = '2015'
            simu  = True
        elif 0 <= fu.find ( '/MC/'    ) : simu = True
        elif 0 <= fu.find ( 'PYTHIA'  ) : simu = True
        elif 0 <= fu.find ( 'BCVEGPY' ) : simu = True


        flst = f.split(' ')
        for _f in flst :
            _ff = _f.strip('"').strip("'")
            ## allow up to 5 symbols for exension 
            first,dot,second = _ff.rpartition('.')
            if dot and 3<=len(second)<=5 and not ext : ext = second.upper() 

    return  dtype,simu,ext

# =============================================================================
##  get file name from filelist 
def fileList  ( file_list ) :
    """
    Get list of files from filelist
    
    """
    if not file_list : return []
    #
    ## try to read filelist 
    import os
    #
    file_list = os.path.expandvars ( file_list ) 
    file_list = os.path.expanduser ( file_list ) 
    file_list = os.path.expandvars ( file_list ) 
    file_list = os.path.expanduser ( file_list ) 
    file_list = os.path.expandvars ( file_list )
    #
    if not os.path.exists ( file_list ) : return []
    result = []
    #
    with open ( file_list , 'r' ) as mfile :
        for line in mfile :
            if line : result.append ( line )
            
    return result 
    

    
# =============================================================================
## try to extract more information from file names 
def hasInFile ( files , pattern ) :
    """
    extract the data type,
    simulaton type and input type from file name 
    """
    #
    if isinstance ( files  , str ) : files = [ files ]
    #
    ptnu = pattern.upper() 
    for f in files  :
        fu = f.upper() 
        if   0 <= fu.find ( ptnu ) : return True
    return False

# =============================================================================
## try to determine the year 
def theYear ( files , params = {} , the_year = None ) :
    """
    Try to determine the year 
    """
    for key in ( 'year' , 'Year' , 'YEAR' ) :
        if params.has_key ( key ) :
            year = params[key] 
            logger.info ("The Year %s is set from params['%s']" % ( year , key ) )
            return year 

    ## try to it get from file-names 
    year,simu,ext = dataType ( files )
    if year :
        logger.info ("The Year %s is set from filenames" % year )
        return year
    
    year = the_year 
    logger.info ("The Year %s is set from default" % year )
    return year 
    
# =============================================================================
if __name__ == '__main__' :

    logger.info ( 80*'*'  ) 
    logger.info ( __doc__ ) 
    logger.info ( ' Author  : %s ' %  __author__  ) 
    logger.info ( ' Version : %s ' %  __version__ ) 
    logger.info ( ' Date    : %s ' %  __date__    ) 
    logger.info ( ' Symbols : %s ' %  list ( __all__ ) ) 
    logger.info ( 80*'*'  ) 

# =============================================================================
# The END 
# =============================================================================

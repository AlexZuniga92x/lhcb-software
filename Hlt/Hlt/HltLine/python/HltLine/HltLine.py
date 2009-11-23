#!/usr/bin/env python
# =============================================================================
# $Id: HltLine.py,v 1.18 2009-11-23 16:50:56 pkoppenb Exp $ 
# =============================================================================
## @file
#
#  simple module which defines few helper classes & utilities
#  for configuration of Hlt(1) lines
#
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-06-22
# =============================================================================
"""
The simple module which defines few configurables for HLT line

The module defines three major public symbols :

 - class    Hlt1Line    :
      simple class to create the proper 'Configurable' for Hlt1 'lines'
 - class    Hlt1Member  :
      helper class to represent the member of Hl1 'line'
 - function hlt1Lines   :
      bookeeping routine which keeps the track of all created Hlt1 'lines'
 - function hlt1Decisions:
      simle function which returns decisions for all created Hlt lines
      
Also few helper symbols are defined:

 - function hlt1Props  :
      simple function which shows the current list of 'major'
      properties/attributes of algorithms to be inspected
 - function addHlt1Prop :
      the function which allows to add more properties/attributes
      into the list of of 'major' properties to be inspected 
 - function rmHlt1Prop :
      the function which allows to remove the properties/attributes
      from the list of of 'major' properties to be inspected 

 - function hlt1InputSelections :
      get tuple of all known input selections
      
 - function hlt1OutputSelections :
      get tuple of all known output selections

 - function hlt1Selections :
      get dictionaty for all known selections:
         'Input'        : all input  selections 
         'Output'       : all output selections 
         'All'          : all input&output selections 
         'Input-Output' : all unmatched input selections 
         'Output-Input' : all unmatched output selections 
         'Input&Output' : all matched selections
         
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = "CVS Tag $Name: not supported by cvs2svn $, $Revision: 1.18 $ "
# =============================================================================

__all__ = ( 'Hlt1Line'     ,  ## the Hlt1 line itself 
            'Hlt2Line'     ,  ## The Hlt2 line itself
            'Hlt1Member'   ,  ## the representation of an Hlt1 line member 
            'Hlt2Member'   ,  ## the representation of an Hlt2 line member 
            'hlt1Lines'    ,  ## list of all created lines 
            'hlt1Decisions',  ## all decisions for created alleys 
            'hlt1Props'    ,  ## list of all major properties for inspection
            'addHlt1Prop'  ,  ## add attribute/property into the list 
            'rmHlt1Prop'   ,  ## remove attribute/property form the list
            'hlt1InputSelections'  , ## the list of all known input selections
            'hlt1OutputSelections' , ## the list of all known output selections
            'hlt1Selections' ) ## the list of all matched selections
            
            

import re
from copy import deepcopy 
from Gaudi.Configuration import GaudiSequencer, Sequencer, Configurable , log
#import logging
#log = logging.getLogger( 'HltLine.HltLine')
#log.setLevel(logging.DEBUG)
from Configurables import DeterministicPrescaler as Scaler
from Configurables import LoKi__L0Filter    as L0Filter
from Configurables import LoKi__HDRFilter   as HDRFilter
from Configurables import LoKi__ODINFilter  as ODINFilter
from Configurables import HltTrackUpgrade   as TrackUpgrade 
from Configurables import HltTrackMatch     as TrackMatch   
from Configurables import HltTrackFilter    as TrackFilter 
from Configurables import HltVertexMaker1   as VertexMaker1 
from Configurables import HltVertexMaker2   as VertexMaker2 
from Configurables import HltVertexFilter   as VertexFilter 
from Configurables import HltVertexUpgrade  as VertexUpgrade 
from Configurables import HltL0MuonCandidates    as L0MuonCandidates 
from Configurables import HltL0CaloCandidates    as L0CaloCandidates 
from Configurables import HltVertexToTracks      as VertexToTracks 
from Configurables import HltAddPhotonToVertex   as AddPhotonToVertex
from Configurables import HltLine                as Line
from Configurables import HltCopySelection_LHCb__Particle_ as HltCopyParticleSelection


## Convention: the name of 'Filter' algorithm inside HltLine
def filterName   ( line , level = 'Hlt1') :
    """Convention: the name of 'Filter' algorithm(s) inside HltLine"""
    return '%s%sFilterSequence'   % (level,line)
## Convention: the name of 'PreScaler' algorithm inside HltLine
def prescalerName  ( line, level = 'Hlt1' ) :
    """ Convention: the name of 'PreScaler' algorithm inside HltLine """
    return '%s%sPreScaler'  % (level,line)
## Convention: the name of 'PostScaler' algorithm inside HltLine
def postscalerName ( line , level = 'Hlt1') :
    """ Convention: the name of 'PostScaler' algorithm inside HltLine """
    return '%s%sPostScaler' % (level, line)
## Convention: the name of 'ODINFilter' algorithm inside HltLine
def odinentryName    ( line, level = 'Hlt1' ) :
    """ Convention: the name of 'ODINFilter' algorithm inside HltLine """
    return '%s%sODINFilter'   % (level,line)
## Convention: the name of 'L0DUFilter' algorithm inside HltLine
def l0entryName    ( line, level = 'Hlt1' ) :
    """ Convention: the name of 'L0DUFilter' algorithm inside HltLine """
    return '%s%sL0DUFilter'   % (level,line)
## Convention: the name of 'HLTFilter' algorithm inside HltLine
def hltentryName    ( line, level = 'Hlt1' ) :
    """ Convention: the name of 'HLTFilter' algorithm inside HltLine """
    return '%s%sHltFilter'   % (level,line)
## Convention: the generic name of 'member' algorithm inside HltLine 
def memberName     ( member, line, level='Hlt1' ) :
    """ Convention: the generic name of 'member' algorithm inside HltLine """
    return '%s%s%s'%(level,line,member.subname())
## Convention: the name of 'Decision' algorithm inside HltLine
def decisionName   ( line, level = 'Hlt1' ) :
    """Convention: the name of 'Decision' algorithm inside HltLine"""
    return level + '%sDecision'   % line if line != 'Global' else level+'Global'

## the list of all created lines 
_hlt_1_lines__ = []
_hlt_2_lines__ = []

def _ordered_lines( lines ) :
    d = {}
    for i in lines : 
        priority = i.priority()
        if not priority : priority = 127
        if priority < 0 or priority > 255 :
            raise AttributeError, 'Priority must by in [0,255], or None, line %s has %s' % (i.name(),i.priority() )
        if priority in d : d[ priority ] += [ i ]
        else             : d[ priority ]  = [ i ]
    l = []
    for i in sorted( d.keys() ) : l += d[i]
    return l
# =============================================================================
## Simple function which returns the (tuple) of all currently created Hlt1Lines
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06
def hlt1Lines () :
    """
    Simple function which returns the (list) of all currently created Hlt1Lines
    
    >>> lines = hlt1Lines()
    >>> print lines
    >>> for line in lines : print line
    
    """
    return _ordered_lines(_hlt_1_lines__)

# =============================================================================
## Simple function which returns the (tuple) of all currently created Hlt2Lines
#  @author Gerhard Raven Gerhard.Raven@nikhef.nl
#  @date   2009-03-27
def hlt2Lines () :
    """
    Simple function which returns the (list) of all currently created Hlt2Lines
    
    >>> lines = hl21Lines()
    >>> print lines
    >>> for line in lines : print line
    
    """
    return _ordered_lines(_hlt_2_lines__)


# =============================================================================
## Simple function whcih returns the decisions for all created Hlt1 lines
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06
def hlt1Decisions () :
    """
    Simple function which returns the decisions for all created Hlt1 lines:
    
    >>> decisions = hlt1Decisions ()
    
    """
    t = [ l.decision() for l in hlt1Lines() if l.decision() ]
    return tuple(t)

# =============================================================================
## Simple function whcih returns the decisions for all created Hlt1 lines
#  @author Gerhard Raven Gerhard.Raven@nikhef.nl
#  @date   2009-03-27
def hlt2Decisions () :
    """
    Simple function which returns the decisions for all created Hlt2 lines:
    
    >>> decisions = hlt2Decisions ()
    
    """
    t = [ l.decision() for l in hlt2Lines() if l.decision() ]
    return tuple(t)

## the list of all input selections
_hlt_1_input_selections__  = []
## the list of all output selections
_hlt_1_output_selections__ = []

# =============================================================================
## get the list of all known input selections
def hlt1InputSelections ( ) :
    """
    Get the full list/tuple of known input selections

    >>> inputs = hlt1InputSelections()
    
    """
    return tuple(_hlt_1_input_selections__)


# =============================================================================
## get the list of all known output selections
def hlt1OutputSelections ( ) :
    """
    Get the full list/tuple of known output selections

    >>> outputs = hlt1OutputSelections()
    
    """
    return tuple(_hlt_1_output_selections__)

# =============================================================================
## add the selection to the list of input selections
def _add_to_hlt1_input_selections_ ( sel ) :
    """
    Add the selection to the list of input selections
    """
    if list is not type(sel) and tuple is not type(sel) : sel = [sel]
    for s in sel :
        if s not in hlt1InputSelections() :
            _hlt_1_input_selections__.append ( s )
    _hlt_1_input_selections__.sort()
    return hlt1InputSelections ()

# =============================================================================
## add the selection to the list of output selections
def _add_to_hlt1_output_selections_ ( sel ) :
    """
    Add the selection to the list of output selections
    """
    if list is not type(sel) and tuple is not type(sel) : sel = [sel]
    for s in sel :
        if s not in hlt1OutputSelections() :
            _hlt_1_output_selections__.append ( s)
    _hlt_1_output_selections__.sort()
    return hlt1OutputSelections ()


# =============================================================================
## get the dictionary of all selections 
def hlt1Selections() :
    """
    Get the dictionary of all selections

    >>> sels = hlt1Selections()

    >>> print sels['All']    # get all selections 
    >>> print sels['Input']  # get input  selections 
    >>> print sels['Output'] # get output selections 
    >>> print sels['Input-Output'] # get unmatched input selections 
    >>> print sels['Output-Input'] # get unmatched output selections 
    >>> print sels['Input&Output'] # get matched input/output selections 
    
    """
    dct = {}
    dct [ 'Input'  ] = hlt1InputSelections ()
    dct [ 'Output' ] = hlt1OutputSelections ()
    import sets
    s1 = sets.Set( hlt1InputSelections  () )
    s2 = sets.Set( hlt1OutputSelections () )
    dct [ 'All' ] = tuple(s1.union(s2) )
    dct [ 'Input&Output' ] = tuple(s1.intersection(s2) )
    dct [ 'Input-Output' ] = tuple(s1.difference(s2))
    dct [ 'Output-Input' ] = tuple(s2.difference(s1))
    return dct
    
# =============================================================================
## Add the created line into the local storage of created Hlt1Lines 
def _add_to_hlt1_lines_( line ) :
    """
    Add the line into the local storage of created Hlt1Lines 
    """
    _hlt_1_lines__.append ( line ) 

# =============================================================================
## Add the created line into the local storage of created Hlt2Lines 
def _add_to_hlt2_lines_( line ) :
    """
    Add the line into the local storage of created Hlt2Lines 
    """
    # actually, want the equivalent of upper_bound on priority...
    _hlt_2_lines__.append ( line ) 
        
# =============================================================================
## the list of possible Hlt1Members types of an Hlt1Line
_types_ = { TrackUpgrade  : 'TU'  
          , TrackMatch    : 'TM'  
          , TrackFilter   : 'TF'  
          , VertexMaker1  : 'VM1' 
          , VertexMaker2  : 'VM2'
          , VertexFilter  : 'VF'
          , VertexToTracks: 'VT'
          , VertexUpgrade : 'VU'
          , L0CaloCandidates : 'L0Calo'
          , L0MuonCandidates : 'L0Muon'
          , AddPhotonToVertex : 'AddPhotonToVertex'
          } 

## protected attributes 
_protected_ = ( 'IgnoreFilterPassed' , 'Members' , 'ModeOR', 'DecisionName', 'Prescale','Postscale','Filter1' )
## own slots for HltLine 
_myslots_   = ( 'name' , 'prescale'  , 'priority', 'postscale' , 'ODIN' , 'L0DU' , 'HLT' , 'algos' , 'PV' ) 

# =============================================================================
## Get the full algorithm type from short nick
def typeFromNick ( nick ) :
    """
    Get the full algorithm type from short nick

    >>> t = typeFromNick ('TU')
    
    """
    for t in _types_ :
        if nick == _types_[t] : return t
    raise AttributeError, " No type is defined for nick '%s'"%nick 

# =============================================================================
## Try to promote a string to the corresponding configurable
def string2Configurable( name ) :
    # try to decode it into something reasonable
    (n,t) = (name,None) if name.find('/') == -1 else name.split('/')
    from Gaudi.Configuration import allConfigurables
    cfg = allConfigurables.get(n)
    if cfg :
        if ( t and cfg.getType() == t ) or cfg.getType() == 'ConfigurableGeneric' :
            return cfg
        else :
            print 'got something for %s, don\'t know what to do with %s'%(name,str(cfg))
            return None
    print 'cannot convert %s into a known configurable' % name
    return None
            
# =============================================================================
## Construct the full selection name
#  The convention is
#     - if the selection name is defined as  'pattern', e.g. '%RZVelo',
#       the full name would include the Hlt line name: 'Hlt1SingleMuonRZVelo'
#     - if the selection name does not include 'pattern', it is unchanged
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06
def _selectionName ( name ,      ## the selection name or pattern 
                     line ,      ## the name of line 
                     level = 'Hlt1') :   ## Hlt1 or Hlt2?
    """
    Construct the full selection name.
    
    The convention is

    - if the selection is of type Hlt1Line or bindMembers, replace it by its output selection

    - if the object is not a string, see if it has either a member function 'outputSelection'
      or an attribute called 'OutputSelection'
    
    - if the selection name is defined as  'pattern', e.g. '%RZVelo',
      the full name would include the Hlt line name: 'Hlt1SingleMuonRZVelo'
      
    - if the selection name does not incldue 'pattern', it is unchanged
    
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06


    >>> _selectionName ( '%RZVelo' , 'SingleMuon' )   #    pattern 
    'Hlt1SingleMuonRZVelo'
    >>> _selectionName ( 'RZVelo'  , 'SingleMuon' )   # no pattern 
    'RZVelo'
    
    """
    if not name : return None
    if type(name) in [ Hlt1Line, bindMembers ] : name = name.outputSelection()
    if type(name) is not str :
        if 'outputSelection' in dir(name)  : name = name.outputSelection()
        if hasattr(name,'OutputSelection') : name = getattr(name,'OutputSelection')
    if '%' != name[0] : return name 
    return level + line + name[1:]


# =============================================================================
## Check the certain selection:
#  - The selection is required to be in the dictionary
#  - If selection has a 'pattern' form , it is expanded properly
#                                 and replaces in the dictionary
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06
def _checkSelection ( sel   ,     ## the selection 
                      margs ,     ## arguments (dictionary)
                      name  ,     ## algorihtm(member) name
                      line  )  :  ## Hlt1 line name 
    """
    Check specified selection:

    - If selection is an Hlt1Line or bindMembers, substitute its output selection

    - If selection has a 'pattern' form (starts from %) ,
          it is expanded properly and the dictionary is updated accordingly

    - TODO: The selection is required to be in the dictionary, 
            and a warning should be given if an input is used which 
            is not contained in the current line (i.e. when the line
            has 'external' dependencies)
    
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06
    
    """
    val = margs.get( sel ,None)
    if not val  :
        raise AttributeError, "'%s' is not specified for %s" % ( sel , name ) 
    val = _selectionName ( val , line )
    # finally redefine the selection name 
    margs[sel] = val 

# =============================================================================
## Check the selections
#    - It require the presence of all mandatory attributes/properties
#    - The mandatory attributes/properties are expanded (if needed)
#    - It requires the absence of 'forbidden' attributes/properties  
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06
def _checkSelections ( args      ,   # the dictionary with arguments 
                       check     ,   # entries to be checked
                       forbidden ,   # forbidden entries
                       name      ,   # the algorithm name
                       line      ) : # the line ID
    """
    Check the selections:
    
    - It require the presence of all mandatory attributes/properties
    
    - The mandatory attributes/properties are expanded (if needed)
    
    - It requires the absence of 'forbidden' attributes/properties
    
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06
    """
    if list is not type(forbidden) and tuple is not type(forbidden) : forbidden =  (forbidden,) 
    for _f in forbidden :
        if _f in args :
            raise ArgumentError, 'The selection "%s" is not allowed for %s' % ( _f , name )
    if list is not type(check)     and tuple is not type(check)     : check     =  (check,) 
    for _c in check : _checkSelection ( _c , args , name , line )

#def _last_algo_with_output( algos ) :
#    print algos
#    for i in  algos : print i
#    for i in range(len(algos)-1,-1,-1) : 
#        if 'OutputSelection' in algos[i].Type.__slots__ : return i
#    return None
# =============================================================================
## Bind members to an HLT line
#  @author Gerhard Raven Gerhard.Raven@nikhef.nl
#  @date   2008-09-16
class bindMembers (object) :
    """
    Simple class to represent a set of Hlt1Members which are bound to a line
    """
    __slots__ = ('_members','_outputsel')

    def outputSelection( self ) : 
        return self._outputsel

    def members( self ) :         
        # remove (downstream) duplicates
        members = []
        for m in self._members :
            if m not in members : members += [ m ]
        return members

    def ignoreOutputSelection( self ) :
        return self.setOutputSelection(None)

    def setOutputSelection( self, outputSelection ) :
        x = deepcopy(self)
        x._outputsel = outputSelection
        return x

#
    def _InputOutputLocationMatchMaker(self,alg) :
        if hasattr(alg,'InputLocations') : # only Hlt2...
            req_inputs = getattr(alg,'InputLocations')
            known_inputs = []
            def _OutputLocationsGetter(alg) :
                    if hasattr(alg,'Members') : 
                        _list = []
                        for i in getattr(alg,'Members') : _list += _OutputLocationsGetter(i) 
                        return _list
                    x =  [ getattr(alg,'OutputLocation') ] if hasattr(alg,'OutputLocation') else [ alg.name() ]
                    return [ getattr(alg,'OutputLocation') ] if hasattr(alg,'OutputLocation') else [ alg.name() ]
            for i in self._members: 
                 known_inputs += _OutputLocationsGetter(i)
            missing = set(req_inputs) - set(known_inputs)
            if missing :
                extra = set(known_inputs) - set(req_inputs) - set(  _OutputLocationsGetter(alg) )
                log.warning( ' input/output matchmaker for  %s generated warnings' %(alg.name()) )
                log.warning( ' ---> missing requests: ' + str(missing)  )
                if extra : 
                    log.warning( ' --->   extra requests: ' + str(extra) )
                    log.warning( ' this might be OK if eg. the extra requests produce the output of the missing requests... but this can not yet be verified automatically')

    def _default_handler_( self, line, alg ) :
        # if not known, blindly copy -- not much else we can do
        self._members += [ alg ]
        # try to guess where the output goes...
#       def _guessOutput( alg ) :
#           def _check(alg) :
#               for attr in [ 'OutputSelection', 'OutputLocation', 'TracksOutContainer', 'OutputTracksName', 'MatchOutput' ] :
#                   if not hasattr( type(alg), attr) : continue
#                   if hasattr( alg, attr) : return getattr( alg, attr )
#               return alg.name()
        if hasattr ( type(alg) , 'OutputSelection' ) :
            if hasattr ( alg , 'OutputSelection' ) :
                self._outputsel = alg.OutputSelection 
        elif hasattr ( type(alg) , 'OutputLocation' ) :
            if hasattr ( alg , 'OutputLocation' ) :
                self._outputsel = alg.OutputLocation 
        elif hasattr ( type(alg) , 'TracksOutContainer' ) :
            if hasattr ( alg , 'TracksOutContainer' ) :
                self._outputsel = alg.TracksOutContainer 
        elif hasattr ( type(alg) , 'OutputTracksName' ) :
            if hasattr ( alg , 'OutputTracksName' ) :
                self._outputsel = alg.OutputTracksName 
        elif hasattr ( type(alg) , 'MatchOutput' ) :
            if hasattr ( alg , 'MatchOutput' ) :
                self._outputsel = alg.MatchOutput 
        else :
            self._outputsel = alg.name()
        self._InputOutputLocationMatchMaker(alg)

    # allow chaining of previously bound members...
    def _handle_bindMembers( self, line, alg ) :
        self._members  += alg.members()
        # do NOT update the current outputselection if
        # the new member doesn't have one...
        if alg.outputSelection() : self._outputsel = alg.outputSelection()

    # if Hlt2Member, ask it to creats a configurable instance for this line..
    # then it's bussines as usual..
    #@TODO: handle expansion/substitution
    def _handle_Hlt2Member( self, line, alg ) :
        if line == None: raise AttributeError, 'Must have a line name to bind to'
        alg = alg.createConfigurable( line, **alg.Args )
        return self._default_handler_( line,  alg )

    # if Hlt1Member, verify, expand, and chain
    def _handle_Hlt1Member( self, line, alg ) :
        if line == None: raise AttributeError, 'Must have a line name to bind to'
        margs = alg.Args.copy() 
        #### TODO: use _checkSelection to make sure the result is valid!!!
        #    expand '%' in FilterDescriptor, InputSelection{,1,2} to allow bound selections
        _subs_cand_ =  set(['FilterDescriptor', 'OutputSelection', 'InputSelections'
                       , 'InputSelection', 'InputSelection1','InputSelection2','InputSelecion3' ])
        for key in set(margs).intersection(_subs_cand_) :
            if type(margs[key]) is str  : margs[key] =   re.sub('%','Hlt1%s'%line,margs[key])
            if type(margs[key]) is list : margs[key] = [ re.sub('%','Hlt1%s'%line,i) for i in margs[key] ]
        algName = alg.name ( line )
        
        ## need input selection?
        if   'InputSelection3' in alg.Type.__slots__ :
            # check input selections
            _checkSelections ( margs ,  # arguments
                               # must be: 
                               [ 'InputSelection3' , 'InputSelection2' , 'InputSelection1' ] ,
                               # not allowed:
                               [ 'InputSelection'  , 'InputSelections' ] ,
                               algName , line ) ;
            #
        elif 'InputSelection2' in alg.Type.__slots__ :
            # check input selections
            _checkSelections ( margs ,  # arguments
                               # must be: 
                               [ 'InputSelection2' , 'InputSelection1' ] ,
                               # not allowed:
                               [ 'InputSelection'  , 'InputSelections' , 'InputSelection3' ] ,
                               algName , line ) ;
            # 
        elif 'InputSelections' in alg.Type.__slots__ :
            # check input selections
            _checkSelections ( margs ,  # arguments
                               # must be: 
                               [ 'InputSelections' ] ,
                               # not allowed:
                               [ 'InputSelection'  , 'InputSelection1' , 'InputSelection2' , 'InputSelection3' ] ,
                               algName , line ) ;
            #
        elif 'InputSelection' in alg.Type.__slots__ :
            # come manual work 
            _inputsel = self._outputsel
            if margs.has_key ( 'InputSelection' ) :
                _checkSelection ( 'InputSelection' , margs , algName , line )
                _inputsel = margs['InputSelection']
            if not _inputsel :
                raise AttributeError, 'Cannot deduce InputSelection neither from argument nor from previous alg'
            margs['InputSelection'] = _inputsel
            # here start the machinery check input selections
            _checkSelections ( margs ,  # arguments
                               # must be: 
                               [ 'InputSelection' ] ,
                               # not allowed:
                               [ 'InputSelections'  , 'InputSelection1' , 'InputSelection2' , 'InputSelection3' ] ,
                               algName , line ) ;
            #
        else :
            log.warning( "Line '%s', Member '%s'  with strange configuration " % ( line , algName ) )

        
        ## output selection (default: the algorithm instance name)
        if 'OutputSelection' in alg.Type.__slots__ : 
            self._outputsel = algName
            if margs.has_key ( 'OutputSelection' ) :
                _checkSelection ( 'OutputSelection' , margs , algName , line ) 
                self._outputsel = margs['OutputSelection']
        
        # create (the configurable for) the algorithm and add it to the sequencer:
        self._members += [ alg.createConfigurable( line , **margs ) ]

    def __init__( self, line, algos ) :
        self._members = []
        self._outputsel = None
        for alg in algos:
            # dispatch according to the type of alg...
            x = '_handle_' + type(alg).__name__
            handle = getattr(self, x if hasattr(self, x) else '_default_handler_')
            handle(line,alg)

# =============================================================================
## @class Hlt1Tool
#  Simple class to represent the settings for a tool used by an Hlt1Member
#  Note: in case of a tool used by a tool, we support recursion...
#  @author Gerhard Raven Gerhard.Raven@nikhef.nl
#  @date   2008-08-23  
class Hlt1Tool (object ) :  
    """
    Simple class to represent the settings for a tool used by an Hlt1Member
    @author Gerhard Raven Gerhard.Raven@nikhef.nl
    @date   2008-08-23  
    """
    __slots__ = ( 'Type' , 'Name' , 'Args', 'Tools' )

    ### The standard constructor to create an Hlt1Tool instance:
    def __init__ ( self        ,    ## ...
                   type = None ,    ## the type of the (configurable corresponding to the) tool
                   name = ''   ,    ## the instance name of the tool
                   tools = []  ,    ## tools used by this tool
                   **Args      ) :  ## other arguments 
        if type  == None : raise AttributeError, "Tool must have a type"
        self.Type = type
        self.Name = name if name else type().getType()
        self.Args = Args
        self.Tools = tools
    
    def createConfigurable( self, parent ) :
        parent.addTool( self.Type, name = self.Name  ) 
        instance = getattr( parent, self.Name )
        for k,v in self.Args.iteritems() : setattr(instance,k,v)
        for tool in self.Tools :  
                if type(tool) is not Hlt1Tool : 
                    raise AttributeError, "The type %s is not an Hlt1Tool"%type(tool)
                tool.createConfigurable( instance )

        
   

# =============================================================================
## @class Hlt1Member
#  Simple class to represent the member of Hlt1 line
#  Only the specific algorithm types are allowed to be "members" of Hlt lines
#  Each such algorithm has a short "type"
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06  
class Hlt1Member ( object ) :
    """
    Simple class to represent the member of Hlt1 line
    Only the specific algorithm types are allowed to be 'members' of Hlt lines
    Each such algorithm has a short 'type'
    
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06
    
    >>> m1 = Member ( 'TU' , 'Velo'   , RecoName = 'Velo' ) # 'HltTrackUpgrade'
    
    """
    __slots__ = ( 'Type' , 'Name' , 'Args', 'Tools', 'CallBack' )
    
    ### The standard constructor to create the  Hlt1Member instance:
    def __init__ ( self       ,    ## ...
                   Type       ,    ## short type of members, e.g. 'TF' HltTrackFilter
                   name  = '' ,    ## the specific part of the algorithm name 
                   tools = [] ,    ## list of tool options for this algorithm
                   callback = None, ## after generating the configurable, call the callback with the generated instance
                   **Args     ) :  ## arguments 
        """
        The standard constructor to create the  Hlt1Member instance:
        
        >>> m1 = Hlt1Member ( 'TU' , 'Velo'   , RecoName = 'Velo' ) ,
        >>> m2 = Hlt1Member ( 'TF' , 'Velo'      ,
                    FilterDescriptor = [ 'IP_PV2D,||>,0.1',
                                         'Calo3DChi2_L0TriggerHadron,<,4' ] ) 
        """
        ## (1) "clone" all agruments
        Type  = deepcopy ( Type  )
        name  = deepcopy ( name  )
        tools = deepcopy ( tools )
        Args  = deepcopy ( Args  )
        callback  = deepcopy ( callback  )
        ##
        Name = name
        if type(Type) is str : Type = typeFromNick ( Type ) 
        if not Type in _types_ :
            raise AttributeError, "The type  %s is not known for Hlt1Member"%Type
        self.Type = Type 
        self.Name = Name 
        self.Args = Args
        self.Tools = tools
        self.CallBack = callback
        for key in Args :
            if  key not in self.Type.__slots__  :
                raise AttributeError, "The key %s is not allowed"%key

    def subtype( self )        :
        " Return the 'subtype' of the member "
        return _types_[ self.Type ]
    def name   ( self , line ) :
        " Return the full name of the member "
        return memberName ( self , line ) 
    def id     ( self )        :
        " Return the ID of the member "        
        return self.subtype() + self.Name
    def subname( self )        :
        " Return the specific part of the name "        
        return self.id()
    def createConfigurable( self, line, **args ) :
        """
        Create the configurable, and, if needed, deal with tool configuration
        """
        ## clone the arguments
        line = deepcopy ( line )
        args = deepcopy ( args ) 
        _name = self.name( line )
        # see if alg has any special Tool requests...
        instance =  self.Type( _name, **args)
        for tool in self.Tools : tool.createConfigurable( instance )
        if self.CallBack : self.CallBack( instance )
        return instance

# ============================================================================
## @class Hl1Line
#  The major class which represent the Hlt1 Line, the sequence.
#
#  The structure of each line is fixed to be
# 
#    - Prescaler
#    - ODINFilter 
#    - L0DUFilter 
#    - HLTFilter
#    - Filter0
#       - Member_1
#       - Member_2
#       - ...
#       - Member_N
#    - Filter1
#       - Member_1
#       - Member_2
#       - ...
#       - Member_N
#    - Postscaler
#    - (implicit) Decision
# 
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06   
class Hlt1Line(object):
    """
    The major class which represent the Hlt1 Line, the sequence.
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06   
    """
    ## The constructor, which defines the line
    #
    #  The major arguments
    #    - 'name'      : short name of the line, e.g, 'SingleMuon'
    #    - 'prescale'  : the prescaler factor
    #    - 'ODIN'      : the list of ODINtype names for ODINFilter 
    #    - 'L0DU'      : the list of L0Channels names for L0Filter 
    #    - 'HLT'       : the list of HLT selections for HLTFilter
    #    - 'algos'     : the list of actual members 
    #    - 'postscale' : the postscale factor
    def __init__ ( self             ,
                   name             ,   # the base name for the Line
                   prescale  = 1    ,   # prescale factor
                   ODIN      = None ,   # ODIN predicate  
                   L0DU      = None ,   # L0DU predicate  
                   HLT       = None ,   # HltDecReports predicate
                   algos     = []   ,   # the list of algorithms/members
                   postscale = 1    ,   # postscale factor
                   priority  = None  ,   # ordering 'hint'
                   **args           ) : # other configuration parameters
        """
        The constructor, which essentially defines the line
        
        The major arguments
        - 'name'      : short name of the line, e.g, 'SingleMuon'
        - 'prescale'  : the prescaler factor
        - 'ODIN'      : the list of ODIN types for ODINFilter
        - 'L0DU'      : the list of L0Channels names for L0Filter
        - 'HLT'       : the list of HLT selections for HLTFilter
        - 'algos'     : the list of actual members 
        - 'postscale' : the postscale factor
        
        """
        ## 1) clone all arguments
        name  = deepcopy ( name  )
        ODIN  = deepcopy ( ODIN  )
        L0DU  = deepcopy ( L0DU  )
        HLT   = deepcopy ( HLT   )
        algos = deepcopy ( algos )
        args  = deepcopy ( args  )
        
        # 2) save all parameters (needed for the proper cloning)
        self._name      = name
        if callable(prescale)  : prescale  = prescale(  self.name() )
        self._prescale  = prescale
        if callable(postscale) : postscale = postscale( self.name() )
        self._postscale = postscale
        self._ODIN      = ODIN
        self._L0DU      = L0DU
        self._HLT       = HLT
        self._priority  = priority
        self._algos     = algos
        self._args      = args

        
        # 3) decision: (pre)set to None, and assign once we're successfully completed ourselfs...
        self._decision  = None 
        
        # check for forbidden attributes
        mdict = {} 
        for key in args :
            if key in _protected_ :
                raise AttributeError, "The attribute'%s' is protected for %s"%(key,self.type())
            mdict[key] = args[key] 

        #start to contruct the sequence        
        line = self.subname()
        
        
        self._outputSelections = []
        # most recent output selection
        self._outputsel = None

        # bind members to line
        _boundMembers = bindMembers( line, algos )
        _members = _boundMembers.members()
        self._outputsel = _boundMembers.outputSelection()
        
        # register selections:
        _input_selection_properties_ = [ 'InputSelection' , 'InputSelection1'
                                       , 'InputSelection2', 'InputSelection3' 
                                       , 'InputSelections'
                                       ] 
        for _m in _members :
            for attr in _input_selection_properties_ :
                if hasattr ( _m , attr  ) :
                    _add_to_hlt1_input_selections_ ( getattr( _m, attr ) )
            if hasattr ( type(_m) , 'OutputSelection' ) :
                if hasattr ( _m , 'OutputSelection' ) :
                    self._outputSelections += [ _m.OutputSelection ]
                    _add_to_hlt1_output_selections_ ( _m.OutputSelection )
                else :
                    self._outputSelections += [ _m.name() ]
                    _add_to_hlt1_output_selections_ ( _m.name         () )

        if self._outputsel is not None and self._outputsel!= decisionName( line ) :
            log.warning( "Line '%s' has a final output selection named '%s'"%(line,self._outputsel) )

        # create the line configurable
        # NOTE: even if pre/postscale = 1, we want the scaler, as we may want to clone configurations
        #       and change them -- and not having the scaler would be problem in that case...
        mdict.update( { 'DecisionName' : decisionName ( line ) 
                      , 'Prescale'     : Scaler(     prescalerName ( line ) , AcceptFraction = self._prescale  )
                      , 'Postscale'    : Scaler(    postscalerName ( line ) , AcceptFraction = self._postscale ) 
                      } )
        if ODIN : mdict.update( { 'ODIN' : ODINFilter ( odinentryName( line ) , Code = self._ODIN )  } )
        if L0DU : 
            from Configurables import L0DUFromRawAlg
            _s = GaudiSequencer( l0entryName(line) + 'Sequence'
                               , Members = [ L0DUFromRawAlg()
                                           , L0Filter( l0entryName( line ) , Code = self._L0DU  )
                                           ] 
                               )
            mdict.update( { 'L0DU' : _s } )
        ## TODO: in case of HLT, we have a dependency... dangerous, as things become order dependent...
        if HLT  : mdict.update( { 'HLT'  : HDRFilter  ( hltentryName ( line ) , Code = self._HLT  ) } )
        if _members : 
            mdict.update( { 'Filter1' : GaudiSequencer( filterName ( line ) , Members = _members ) })
        # final cloning of all parameters:
        __mdict = deepcopy ( mdict ) 
        self._configurable = Line ( self.name() , **__mdict )

        # fix numbering scheme of line members
        self._index = computeIndices( self._configurable )

        ## finally assign the decision name!
        self._decision = decisionName ( line )

        # register into the local storage of all created Lines
        _add_to_hlt1_lines_( self ) 

    ## 'sub-name' of Hlt Line 
    def subname   ( self ) :
        """ 'Sub-name' of the Hlt line  """ 
        return            self._name
    ## Full name of Hlt line 
    def name      ( self ) :
        """ Full name of Hlt Line """
        return 'Hlt1%s' % self._name
    ## the actual type of Hlt Line 
    def type      ( self ) :
        """ The actual type of Hlt Line """
        return Hlt1Line
    ## Get the underlying 'Configurable'
    #  probably it is the most important method except the constructor
    #
    #  @code 
    #  >>> line = Hlt1Line ( .... )
    #  >>> conf = line.sequencer()
    #  @endcode
    def sequencer ( self ) :
        """
        Get the underlying 'Configurable' instance 
        probably it is the most important method except the constructor
        
        >>> line = Hlt1Line ( .... )
        >>> conf = line.sequencer()
        
        """
        return self._configurable
    ## Get the underlying 'Configurable'
    #  probably it is the most important method except the constructor
    #
    #  @code 
    #  >>> line = Hlt1Line ( .... )
    #  >>> conf = line.configurable() 
    #  @endcode    
    def configurable ( self ) :
        """
        Get the underlying 'Configurable' instance 
        probably it is the most important method except the constructor
        
        >>> line = Hlt1Line ( .... )
        >>> conf = line.configurable()
        """
        return self.sequencer()
    ## get the lits of all 'IDs' for all members 
    def ids ( self ) :
        """ Get the list of all IDs for all members """
        _ids = [] 
        for alg in self._algos :
            if Hlt1Member is type(alg) : _ids += [ alg.id() ] 
        return _ids
    ## get the decision of the line
    def decision ( self ) :
        """
        Get the actual decision of the line

        >>> line = ...
        >>> decision = line.decision()
        
        """
        if not self._decision :
            raise AttributeError, "The line %s does not define valid decision " % self.subname()
        return self._decision
    ## get the last output selection of the line
    def outputSelection ( self ) :
        """
        Get the name of last outputSelection of the line

        >>> line = ...
        >>> selection = line.outputSelection()
        
        """
        if not self._outputsel :
            raise AttributeError, "The line %s does not define valid outputSelection " % self.subname()
        return self._outputsel

    def outputSelections ( self ) :
        return self._outputSelections


    # determine the index for the given algorithm name
    def index( self, name = None ) :
        if name :
            return self._index.index(name) if name in self._index else None
        else :
            return self._index

    # ordering hint
    def priority( self ) :
        return self._priority
    
    ## Clone the line  
    def clone ( self , name , **args ) :
        """
        Clone the line
        
        A new Hlt Line is created with new name, all property/attrributes maps
        are updated accordingly.
        
        """
        # add some python magic to allow reasonable definition of the deepcopy 
        # of a member function bound to an object instance.
        # see http://bugs.python.org/issue1515 for more info...
        # This should be fixed in python 2.6, so at some point this hack
        # can be removed again
        import copy,types
        origMethod = copy._deepcopy_dispatch[types.MethodType] if types.MethodType in copy._deepcopy_dispatch else None
        def _deepcopy_method(x, memo):
            return type(x)(x.im_func, deepcopy(x.im_self, memo), x.im_class)
        copy._deepcopy_dispatch[types.MethodType] = _deepcopy_method


        ## 1) clone the arguyments
        args = deepcopy ( args )

        ## 2) classify arguments:
        _own   = {} # own arguments 
        _seq   = {} # arguments for sequencer
        _other = {} # the rest (probably reconfiguration of members)
        for key in args :
            if    key in GaudiSequencer.__slots__ : _seq   [keq] = args[key]
            elif  key in  _myslots_               : _own   [key] = args[key] 
            else                                  : _other [key] = args[key]

        # Explictly copy all major structural parameters 
        __name       = deepcopy ( name        )
        __prescale   = deepcopy ( args.get ( 'prescale'  , self._prescale  ) ) 
        __ODIN       = deepcopy ( args.get ( 'ODIN'      , self._ODIN      ) )        
        __L0DU       = deepcopy ( args.get ( 'L0DU'      , self._L0DU      ) )        
        __HLT        = deepcopy ( args.get ( 'HLT'       , self._HLT       ) )        
        __postscale  = deepcopy ( args.get ( 'postscale' , self._postscale ) ) 
        __algos      = deepcopy ( args.get ( 'algos'     , self._algos     ) )
        __priority   = deepcopy ( args.get ( 'priority'  , self._priority  ) )
        __args       = deepcopy ( self._args  ) 

        # restore the original deepcopy behaviour...
        if origMethod :
            copy._deepcopy_dispatch[types.MethodType] = origMethod 
        else :
            del copy._deepcopy_dispatch[types.MethodType]

        # Check the parameters, reponsible for reconfiguration:
        for alg in [ i for i in __algos if type(i) is Hlt1Member ] :
            id = alg.id()
            if id in _other :
                alg.Args.update( _other [id] ) 
                del _other [id]

        # unknown parameters/arguments 
        if _other :
            raise AttributeError, 'Invalid attributes are detected: %s'%_other 

        # get all "OLD" arguments
        # and update them with all arguments, understandable by Sequencer 
        __args.update ( _seq   )

        return Hlt1Line ( name      = __name       ,
                          prescale  = __prescale   ,
                          ODIN      = __ODIN       ,
                          L0DU      = __L0DU       ,
                          HLT       = __HLT        ,
                          postscale = __postscale  ,
                          priority  = __priority   ,
                          algos     = __algos      , **__args )
    
    
# =============================================================================
## @class Hlt2Member
#  Simple class to represent the member of Hlt2 line
#  Only the specific algorithm types are allowed to be "members" of Hlt lines
#  Each such algorithm has a short "type"
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06  
class Hlt2Member ( object ) :
    """
    Simple class to represent the member of Hlt2 line
    Only some specific algorithm types are allowed to be 'members' of Hlt lines
    
    @author Gerhard Raven Gerhard.Raven@nikhef.nl
    based on the original Hlt1Member by
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2008-08-06
    
    """
    __slots__ = ( 'Type' , 'Name' , 'Args', 'Tools' )
    
    ### The standard constructor to create the  Hlt1Member instance:
    def __init__ ( self       ,    ## ...
                   Type       ,    ## type of members
                   name  = '' ,    ## the specific part of the algorithm name 
                   tools = [] ,    ## list of tool options for this algorithm
                   **Args     ) :  ## arguments 
        """
        The standard constructor to create the  Hlt1Member instance:
        >>> m1 = Hlt2Member ( FilterDesktop , 'Filter', Code = '...', InputLocations = ... ,
        """
        from Configurables import FilterDesktop, CombineParticles
        ## (0) verify input
        # Type must be a (configurable) class name, and only
        # a limited set is allowed (which must be DVAlgorithms...)
        if Type not in [ FilterDesktop, CombineParticles ] :
            raise AttributeError, "The type  %s is not known for Hlt2Member"%Type
        for key in Args :
            if  key not in Type.__slots__  :
                raise AttributeError, "The key %s is not allowed for type %s"%(key,Type.__name__)

        ## (1) "clone" all agruments
        self.Type  = deepcopy ( Type  )
        self.Name  = deepcopy ( name  )
        self.Args  = deepcopy ( Args  )
        self.Tools = deepcopy ( tools )

    def clone( self, name, **mods ) :
        args = deepcopy( self.Args )
        args.update( mods )
        return Hlt2Member( self.Type
                         , name = name
                         , tools = self.Tools
                         , **args )

    def subtype( self )        :
        " Return the 'subtype' of the member "
        return self.Type.__name__
    def name   ( self , line ) :
        " Return the full name of the member "
        return memberName ( self , line, level = 'Hlt2' ) 
    def id     ( self )        :
        " Return the ID of the member "        
        return self.Name
    def subname( self )        :
        " Return the specific part of the name "        
        return self.id()
    def createConfigurable( self, line, **args ) :
        """
        Create the configurable, and, if needed, deal with tool configuration
        """
        ## clone the arguments
        line = deepcopy ( line )
        args = deepcopy ( args ) 
        if 'InputLocations' in args : 
            # adapt input...  and put back...  
            inputLocations = args.pop('InputLocations')
            def _adapt(i,line) :
                from Configurables import FilterDesktop, CombineParticles
                if type(i) is bindMembers :
                       return  i.outputSelection()
                elif type(i) in [ CombineParticles, FilterDesktop ] :
                       return i.getName()
                elif type(i) is Hlt2Member :
                       return 'Hlt2'+line+i.subname()
                else :
                       from re import sub
                       return sub('^%', 'Hlt2' + line, i )
            inputLocations = [ _adapt(i,line) for i in inputLocations ]
            # deal with nested lists (one level only), keep order invariant
            args['InputLocations'] = []
            for i in inputLocations :
                args['InputLocations']  += i if type(i) is list else [ i ]

        _name = self.name( line )
        instance =  self.Type( _name, **args)
        # see if alg has any special Tool requests...
        for tool in self.Tools : tool.createConfigurable( instance )
        return instance

# ============================================================================
## @class Hlt2Line
#  The major class which represent the Hlt2 Line, the sequence.
#
#  The structure of each line is fixed to be
# 
#    - Prescaler
#    - ODINFilter 
#    - L0DUFilter 
#    - HLTFilter
#    - Filter0
#       - Member_1
#       - Member_2
#       - ...
#       - Member_N
#    - Filter1
#       - Member_1
#       - Member_2
#       - ...
#       - Member_N
#    - Postscaler
#    - (implicit) Decision
# 
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-08-06   
class Hlt2Line(object):
    """
    The major class which represent the Hlt2 Line, the sequence.
    @author Gerhard Raven, Gerhard.Raven@nikher.nl,
    based on the original Hlt1Line by
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date   2009-03-25   
    """

    _PVAlgorithms = None
    _RecoAlgorithms = None
    ## The constructor, which defines the line
    #
    #  The major arguments
    #    - 'name'      : short name of the line, e.g, 'SingleMuon'
    #    - 'prescale'  : the prescaler factor
    #    - 'ODIN'      : the list of ODINtype names for ODINFilter 
    #    - 'L0DU'      : the list of L0Channels names for L0Filter 
    #    - 'HLT'       : the list of HLT selections for HLTFilter
    #    - 'Reco'      : insert reconstruction (or not)
    #    - 'PV'        : insert PV reconstruction (or not)
    #    - 'algos'     : the list of actual members 
    #    - 'postscale' : the postscale factor
    def __init__ ( self             ,
                   name             ,   # the base name for the Line
                   prescale  = 1    ,   # prescale factor
                   ODIN      = None ,   # ODIN predicate
                   L0DU      = None ,   # L0DU predicate  
                   HLT       = None ,   # HltDecReports predicate
                   algos     = []   ,   # the list of algorithms/members
                   postscale = 1    ,   # postscale factor
                   priority  = None ,   # hint for ordering lines
                   PV        = None ,   # insert PV reconstruction at the start of the line
                   Reco      = True ,   # request Hlt2 reconstruction sequence
                   **args           ) : # other configuration parameters
        """
        The constructor, which essentially defines the line
        
        The major arguments
        - 'name'      : short name of the line, e.g, 'SingleMuon'
        - 'prescale'  : the prescaler factor
        - 'ODIN'      : the list of ODIN types for ODINFilter
        - 'L0DU'      : the list of L0Channels names for L0Filter
        - 'HLT'       : the list of HLT selections for HLTFilter
        - 'algos'     : the list of actual members 
        - 'postscale' : the postscale factor
        
        """
        if type(Reco) != bool : raise AttributeError, "Must specify Reco = True or Reco = False when constructing  Hlt2Line %s"%(name)
        if not self._RecoAlgorithms :
            ## TODO: push this into the individual dependency chains of eg. NoCutsPions, ...
            #  Full reconstruction of all tracks 
            from HltReco import HltRecoSequence
            self._RecoAlgorithms = [ HltRecoSequence  ]

        if type(PV) != bool : raise AttributeError, "Must specify PV = True or PV = False when constructing  Hlt2Line %s"%(name)
        if not self._PVAlgorithms : 
            from HltReco import PV3D
            self._PVAlgorithms = PV3D.members()

        if not HLT :
            HLT = "HLT_PASS_RE('Hlt1(?!Lumi).*Decision')"

        ## 1) clone all arguments
        name  = deepcopy ( name  )
        ODIN  = deepcopy ( ODIN  )
        L0DU  = deepcopy ( L0DU  )
        HLT   = deepcopy ( HLT   )
        PV    = deepcopy ( PV    )
        Reco  = deepcopy ( Reco  )
        algos = deepcopy ( algos )
        args  = deepcopy ( args  )
        
        # 2) save all parameters (needed for the proper cloning)
        self._name      = name
        if callable(prescale) : prescale = prescale( self.name() )
        self._prescale  = prescale
        if callable(postscale) : postscale = postscale( self.name() )
        self._postscale = postscale
        self._priority  = priority
        self._ODIN      = ODIN
        self._L0DU      = L0DU
        self._HLT       = HLT
        self._PV        = PV
        self._Reco      = Reco
        self._algos     = algos
        self._args      = args

        
        # 3) decision: (pre)set to None, and assign once we're successfully completed ourselfs...
        self._decision  = None 
        
        # check for forbidden attributes
        mdict = {} 
        for key in args :
            if key in _protected_ :
                raise AttributeError, "The attribute'%s' is protected for %s"%(key,self.type())
            mdict[key] = args[key] 

        #start to contruct the sequence        
        line = self.subname()

        # bind members to line 
        xalgos = []
        if self._Reco : xalgos += self._RecoAlgorithms
        if self._PV   : xalgos += self._PVAlgorithms 
        xalgos += algos 
        _boundMembers = bindMembers( line, xalgos )
        _members = _boundMembers.members()


        # create the line configurable
        # NOTE: even if pre/postscale = 1, we want the scaler, as we may want to clone configurations
        #       and change them -- and not having the scaler would be problem in that case...
        mdict.update( { 'DecisionName' : decisionName ( line, 'Hlt2' ) 
                      , 'Prescale'     : Scaler(     prescalerName ( line,'Hlt2' ) , AcceptFraction = self._prescale  )
                      , 'Postscale'    : Scaler(    postscalerName ( line,'Hlt2' ) , AcceptFraction = self._postscale ) 
                      } )
        if ODIN : mdict.update( { 'ODIN' : ODINFilter ( odinentryName( line,'Hlt2' ) , Code = self._ODIN )  } )
        if L0DU : mdict.update( { 'L0DU' : L0Filter   ( l0entryName  ( line,'Hlt2' ) , Code = self._L0DU )  } )
        ## TODO: in case of HLT, we have a dependency... dangerous, as things become order dependent...
        if HLT  : mdict.update( { 'HLT'  : HDRFilter  ( hltentryName ( line,'Hlt2' ) , Code = self._HLT  ) } )
        if _members : 
            last = _members[-1]
            while hasattr(last,'Members') : 
                last = getattr(last,'Members')[-1]
            ## TODO: check if 'last' is a FilterDesktop, CombineParticles, or something else...
            needsCopy = [ 'CombineParticles', 'FilterDesktop', 'Hlt2DisplVertices' ]
            knownLastMembers = needsCopy + [ 'HltCopySelection<LHCb::Track>' ]
            if last.getType() not in knownLastMembers :
              log.warning( 'last item in line ' + self.name() + ' is ' + last.getName() + ' with type ' + last.getType() )
            members = _members
            if last.getType() in needsCopy :
                members += [ HltCopyParticleSelection( decisionName( line, 'Hlt2')
                                                     , InputSelection = 'TES:/Event/HLT/%s/Particles'%last.getName()
                                                     , OutputSelection = decisionName(line, 'Hlt2')) ]
            mdict.update( { 'Filter1' : GaudiSequencer( filterName ( line,'Hlt2' ) , Members = members ) })
        # final cloning of all parameters:
        __mdict = deepcopy ( mdict ) 
        self._configurable = Line ( self.name() , **__mdict )

        # fix numbering scheme of line members
        self._index = computeIndices( self._configurable )

        # put upper limit on combinatorics
        limitCombinatorics( self._configurable, maxCandidates = 2000  ) 

        ## finally assign the decision name!
        self._decision = decisionName ( line, 'Hlt2' )

        # register into the local storage of all created Lines
        _add_to_hlt2_lines_( self ) 

    ## 'sub-name' of Hlt Line 
    def subname   ( self ) :
        """ 'Sub-name' of the Hlt line  """ 
        return            self._name
    ## Full name of Hlt line 
    def name      ( self ) :
        """ Full name of Hlt Line """
        return 'Hlt2%s' % self._name
    ## the actual type of Hlt Line 
    def type      ( self ) :
        """ The actual type of Hlt Line """
        return Hlt2Line
    ## Get the underlying 'Configurable'
    #  probably it is the most important method except the constructor
    #
    #  @code 
    #  >>> line = Hlt2Line ( .... )
    #  >>> conf = line.configurable() 
    #  @endcode    
    def configurable ( self ) :
        """
        Get the underlying 'Configurable' instance 
        probably it is the most important method except the constructor
        
        >>> line = Hlt1Line ( .... )
        >>> conf = line.configurable()
        """
        return self._configurable
    ## get the decision of the line
    def decision ( self ) :
        """
        Get the actual decision of the line

        >>> line = ...
        >>> decision = line.decision()
        
        """
        if not self._decision :
            raise AttributeError, "The line %s does not define valid decision " % self.subname()
        return self._decision

    # determine the index for the given algorithm name
    def index( self, name ) :
        return self._index.index(name) if name in self._index else None

    # ordering hint -- WARNING: think twice before using this. Lines _must_ be selfcontained
    # (with only a few exceptions). So do not assume any ordering when you write a line
    def priority( self ) :
        return self._priority

    ## Clone the line  
    def clone ( self , name , **args ) :
        """
        Clone the line
        
        A new Hlt Line is created with new name, all property/attrributes maps
        are updated accordingly.
        
        """
        # add some python magic to allow reasonable definition of the deepcopy 
        # of a member function bound to an object instance.
        # see http://bugs.python.org/issue1515 for more info...
        # This should be fixed in python 2.6, so at some point this hack
        # can be removed again
        import copy,types
        origMethod = copy._deepcopy_dispatch[types.MethodType] if types.MethodType in copy._deepcopy_dispatch else None
        def _deepcopy_method(x, memo):
            return type(x)(x.im_func, deepcopy(x.im_self, memo), x.im_class)
        copy._deepcopy_dispatch[types.MethodType] = _deepcopy_method


        ## 1) clone the arguyments
        args = deepcopy ( args )

        ## 2) classify arguments:
        _own   = {} # own arguments 
        _seq   = {} # arguments for sequencer
        _other = {} # the rest (probably reconfiguration of members)
        for key in args :
            if    key in GaudiSequencer.__slots__ : _seq   [keq] = args[key]
            elif  key in  _myslots_               : _own   [key] = args[key] 
            else                                  : _other [key] = args[key]

        # Explictly copy all major structural parameters 
        __name       = deepcopy ( name        )
        __prescale   = deepcopy ( args.get ( 'prescale'  , self._prescale  ) ) 
        __ODIN       = deepcopy ( args.get ( 'ODIN'      , self._ODIN      ) )        
        __L0DU       = deepcopy ( args.get ( 'L0DU'      , self._L0DU      ) )        
        __HLT        = deepcopy ( args.get ( 'HLT'       , self._HLT       ) )        
        __PV         = deepcopy ( args.get ( 'PV'        , self._PV        ) )        
        __postscale  = deepcopy ( args.get ( 'postscale' , self._postscale ) ) 
        __priority   = deepcopy ( args.get ( 'priority ' , self._priority  ) ) 
        __algos      = deepcopy ( args.get ( 'algos'     , self._algos     ) )
        __args       = deepcopy ( self._args  ) 

        # restore the original deepcopy behaviour...
        if origMethod :
            copy._deepcopy_dispatch[types.MethodType] = origMethod 
        else :
            del copy._deepcopy_dispatch[types.MethodType]

        # Check the parameters, reponsible for reconfiguration:
        for alg in [ i for i in __algos if type(i) is Hlt2Member ] :
            id = alg.id()
            if id in _other :
                 alg.Args.update( _other [id] ) 
                 del _other [id]

        # unknown parameters/arguments 
        if _other :
            raise AttributeError, 'Invalid attributes are detected: %s'%_other 

        # get all "OLD" arguments
        # and update them with all arguments, understandable by Sequencer 
        __args.update ( _seq   )

        return Hlt2Line ( name      = __name       ,
                          prescale  = __prescale   ,
                          ODIN      = __ODIN       ,
                          L0DU      = __L0DU       ,
                          HLT       = __HLT        ,
                          PV        = __PV         ,
                          postscale = __postscale  ,
                          priority  = __priority   ,
                          algos     = __algos      , **__args )
    
#
#
#
def computeIndices( configurable ) :
    # note: for decisions, we can have positive sequencer, with negative members
    #       (OR, or ignoreFilterPassed)
    #       and if sequencer pass, then no need to check its members..
    #       But if the aim is to 'number' selections, there is no reason
    #       to keep the sequencer... On the other hand, we should have 
    #       a single scheme (both for keeping tracks where we stopped,
    #       and for selections), and being 'sparse' is not a problem...
    #       For decisions, the seqeuencer depends on its members,
    #       so logically, the sequencer comes _after_ the members
    # As a result, we do a 'depth first' traversal of the tree.
    list = []
    if hasattr( configurable, 'Members' ) :
        for i in getattr( configurable, 'Members' ) : 
            list += computeIndices(i)
    elif type(configurable) == Line :
        stages = [ 'Prescale','ODIN','L0DU','HLT','Filter0','Filter1','Postscale' ]
        for i in [ getattr( configurable, j ) for j in stages if hasattr(configurable,j) ] :
            list +=  computeIndices( i )
    list += [ configurable.name() ]
    return list

def limitCombinatorics( configurable, maxCandidates, incidentName = 'ExceedsCombinatoricsLimit' ) :
    val = False
    from Configurables import CombineParticles
    if hasattr( configurable, 'Members' ) :
        for i in getattr( configurable, 'Members' ) : 
            # order is important to avoid shortcircuit from skipping call to limitCombinatorics!
            val = limitCombinatorics( i, maxCandidates, incidentName ) or val
        return val
    elif type(configurable) == Line :
        stages = [ 'Filter0','Filter1' ]
        for i in [ getattr( configurable, j ) for j in stages if hasattr(configurable,j) ] :
            # order is important to avoid shortcircuit from skipping call to limitCombinatorics!
            val = limitCombinatorics( i, maxCandidates, incidentName ) or val
        if val : configurable.IncidentsToBeFlagged += [ incidentName ]
        return val
    elif type(configurable) == CombineParticles :
        configurable.StopAtMaxCandidates = True
        configurable.MaxCandidates       = maxCandidates
        configurable.StopIncidentType    = incidentName
        return True

    
# =============================================================================
# Some useful decorations 
# =============================================================================
## Calculate the effective length of the string
#  which is the length from the last '\n'-symbol 
def len1 ( line ) :
    """
    Calculate the effective length of the line,
    which is thw length from the last '\n'-symbol 
    """
    _i = line.rfind('\n')
    return len(line) if _i < 0 else len(line) - _i

## the major properties/attributes  
_hlt1_props_   = [ 'AcceptFraction'    , 
                   'PercertPass'       ,
                   'L0Channels'        ,
                   'InputSelection'    ,
                   'InputSelection1'   ,
                   'InputSelection2'   ,
                   'InputSelection3'   ,
                   'InputSelections'   ,
                   #'OutputSelection'   ,
                   'RecoName'          ,
                   'MatchName'         ,
                   'FilterDescription' ] 

## Get the tuple of major interesting properties for Hlt1  objects
def hlt1Props () :
    """
    Get the tuple of major interesting properties for Hlt1  objects

    >>> print hlt1Props()
    
    """
    _hlt1_props_.sort() 
    return tuple(_hlt1_props_)

## Add the item into the list of Hlt major proeprties 
def addHlt1Prop ( prop ) :
    """
    Add the item into the list of Hlt major proeprties
    
    >>> print hlt1Props ()
    >>> print addHlt1Prop ( 'OutputLevel' )
    
    """
    if  not list is type(prop) and not  tuple is type(prop) : prop = [ prop ] 
    # loop over all items:
    for item in prop :
        if not item in _hlt1_props_ : _hlt1_props_.append ( item )
    # 
    return hlt1Props()

## remove the item from the list of Hlt1 major properties
def rmHlt1Prop ( prop ) :
    """
    Remove the item into the list of Hlt major proeprties
    
    >>> print hlt1Props ()
    >>> rmHlt1Prop ( 'OutputLevel' )
    
    """
    if  not list is type(prop) and not  tuple is type(prop) : prop = [ prop ]
    for item in prop :
        if not item in _hlt1_props_ : return hlt1Props() 
        pos = _hlt1_props_.index ( item )
        del _hlt1_props_[ pos ]
    return hlt1Props() 

# =============================================================================
## Print the major properties/attributes of the configurables
#  @param obj  the object to be inspected
#  @param lst  list of attributes/properties 
#  @param line the (output) line
#  @paral l1   the indentation parameter
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-08-06
def prnt ( obj        ,   # the object 
           lst  = []  ,   # list of 'major' properties
           line = ''  ,   # the line to eb updated
           l1   = 65  ) : # the indentation/format parameter 
    """
    Print the major properties/attributes of the configurables
    @param obj  the object to be inspected
    @param lst  list of attributes/properties 
    @param line the (output) line
    @paral l1   the indentation parameter
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date 2008-08-06

    >>> obj = ...  # get the object/configurable
    >>> print prnt ( obj , [ 'OutputLevel' , 'Members'] ) # print... 
    
    """
    if not lst : lst = hlt1Props () 
    elif list  is type(lst) : pass
    elif tuple is type(lst) : pass
    else : lst = [ lst ]
    #
    for item in lst :
        if hasattr ( obj , item ) :
            if l1 < len1( line ) : line += '\n' + l1*' '
            line += "%-15s : %s" % ( item , getattr ( obj , item ) )
    return line 

# =============================================================================
## The helper function for narcissic self-print of sequences  & algorithms 
#  @param self  the object to be inspected
#  @param level the recursion level
#  @param lst   the list of major properties/attributes 
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-08-06
def __enroll__ ( self       ,   ## the object
                 level = 0  ,   ## the recursion level
                 lst   = [] ) : ## the major properties  
    """
    The helper function for narcissic self-print of sequences  & algorithms 
    @param self  the object to be inspected
    @param level the recursion level
    @param lst   the list of major properties/attributes 
    @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
    @date 2008-08-06
    
    """

    if type(self) == str :
        cfg = string2Configurable(self)
        if cfg : self = cfg
    if hasattr ( self , 'sequencer' ) :
        return __enroll__ ( self.sequencer() , level )

    _tab = 50
    _indent_ = ('%-3d'%level) + level * '   ' 
    try:     line = _indent_ + self.name ()
    except:  line = _indent_ + ( self if type(self) == str else '<UNKNOWN>' )
        
    if len1(line)>( _tab-1): line +=  '\n'+ _tab*' '
    else :                   line +=  (_tab-len1(line))*' '
    try:                     line +=  '%-25.25s'%self.getType()
    except:                  line +=  '<UNKNOWN>'

    line = prnt ( self , lst , line, l1 = _tab+25 ) + '\n'
    
    # use the recursion 
    if hasattr ( self , 'Members' ) :
        for _m in getattr(self,'Members') : line += __enroll__ ( _m , level + 1 , lst ) 

    if type(self) is Line :
        for i in [ 'Prescale','ODIN','L0DU','HLT','Filter0','Filter1','Postscale' ] :
            if hasattr(self,i) : line += __enroll__( getattr(self,i), level + 1, lst )

    return line


Hlt1Line       . __str__ = __enroll__    
Line           . __str__ = __enroll__ 
GaudiSequencer . __str__ = __enroll__ 
Sequencer      . __str__ = __enroll__ 

# =============================================================================
## The simple representation of Hlt1Line
def __line1_repr__ ( self , *args ) :
    """
    The simple representation of Hlt1Line
    """
    return "Hlt1Line('%s')" % self.name()

Hlt1Line . __repr__ = __line1_repr__ 

# =============================================================================
## The simple representation of Hlt1Line
def __line2_repr__ ( self , *args ) :
    """
    The simple representation of Hlt1Line
    """
    return "Hlt2Line('%s')" % self.name()

Hlt2Line . __repr__ = __line2_repr__ 

# =============================================================================
## some embedded action
if '__main__' == __name__ :
    print __doc__
    print __version__
    print __author__
    print "\n\tSymbols:\n", dir ()
    

# =============================================================================
# The END 
# =============================================================================

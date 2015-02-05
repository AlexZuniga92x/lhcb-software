#########################################################################################
# Utility function for setting thresholds both in Hlt1 and 2
#
def setThresholds(ThresholdSettings,conf_):
    """
    Look in ThresholdSettings for configurable conf_
    and set the appropriate settings
    
    @author G. Raven, P. Koppenburg
    @date 23/7/2009 (moved)
    """
    from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
    assert issubclass(conf_,HltLinesConfigurableUser) , '%s is not an HltLinesConfigurableUser'%i.__name__
    conf = conf_()  # The configurable _must_ be instantiated even if not explicitly configured. Or it will be ignored
    for (k,v) in ThresholdSettings.get(conf_, dict()).iteritems() :
        if hasattr(conf,k) and v==getattr(conf,k) : continue # nothing to do, move on to next item...

        # configurables have an exception for list and dict: 
        #   even if not explicitly set, if you ask for them, you get one...
        #   this is done to make foo().somelist += ... work.
        # hence we _assume_ that, even if we have an attr, but it matches the
        # default, it wasn't set explicitly, and we overrule it...
        if hasattr(conf,k) and conf.getProp(k) != conf.getDefaultProperty(k) :
                from Gaudi.Configuration import log
                log.warning('%s.%s has explicitly been set, NOT using requested predefined threshold %s, but keeping explicit value: %s '%(conf.name(),k,str(v),getattr(conf,k)))
        else :
            if ( type(v) == dict ): # special case for dictionaries (needed in topo)
                val = conf.getProp(k)
                val.update(v)
                setattr(conf,k,val)
            else :
                setattr(conf,k,v)


def Name2Threshold(name) :
    if not hasattr(Name2Threshold,'_dict') : Name2Threshold._dict = {}
    if name not in Name2Threshold._dict : 
        from Gaudi.Configuration import log
        log.warning(' '+'#'*(41+len(name)) )
        log.warning(' ## using trigger threshold settings "%s" ##'%name)
        try :
            mod = getattr( __import__('HltSettings.%s' % name ), name )
            cls = getattr( mod, name )
        except (AttributeError,ImportError) as error:
            log.error('   ## Unknown ThresholdSetting "%s" -- please fix Moore().ThresholdSetting ##' % name )
            raise error
        Name2Threshold._dict[name] = cls()
        #log.warning(' ## type is %-30s ##' % Name2Threshold._dict[name].HltType() )
        log.warning(' '+'#'*(41+len(name)) )
    return Name2Threshold._dict[name]

def importLineConfigurables(pkg, endsWithLines = False) :
    # import all modules in Hlt{1,2}Lines, and require each file xyz to contain a class xyzConf
    # i.e. do the equivalent of 
    #    import HltXLines.HltXSomeLines, ...
    #    return [ HltXLines.HltXSomeLines.HltXSomeLinesConf , ... ]
    #
    import os.path, pkgutil, importlib
    old_style = [getattr(importlib.import_module(pkg.__name__ + '.' + name), name + 'Conf')
                 for _, name, is_pkg in pkgutil.iter_modules(pkg.__path__)
                 if (not endsWithLines or (endsWithLines and name.endswith('Lines'))) and not is_pkg]
    new_style = [getattr(importlib.import_module(pkg.__name__ + '.' + name + '.Lines'), name + 'Lines')
                 for _, name, is_pkg in pkgutil.iter_modules(pkg.__path__)
                 if is_pkg if name not in ('Utilities')]
    return old_style + new_style

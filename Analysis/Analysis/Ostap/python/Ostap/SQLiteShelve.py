#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
# $Id$
# =============================================================================
# @file SQLiteShelve.py
# 
# This is SQLite-version of shelve database with compressed content 
# 
# Keeping the same interface and functionlity as shelve data base,
# SQLiteShelve allows much more compact file size through
# the compression of the content
#
# The actual code has been taken from <c>sqlitedict</c> code
#  by Radim Rehurek <radimrehurek@seznam.cz>
# Hacked together from:
#  * http://code.activestate.com/recipes/576638-draft-for-an-sqlite3-based-dbm/
#  * http://code.activestate.com/recipes/526618/   ( see Google...)
#
# The compression (with zlib) is added atop of original code 
# 
# Create new DB:
#
# @code
#
# >>> import SQLiteShelve                     ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db', 'n')    ## create new DB
# ...
# >>> abcde = ...
# >>> db['some_key'] =  abcde                 ## add information to DB
# ...
# >>> db.close()
#
# @endcode 
#
# Access to DB in read-only mode :
#
# @code
#
# >>> import SQLiteShelve                    ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db' , 'r' ) ## access existing dbase in read-only mode
# ...
# >>> for key in db : print key
# ...
# >>> abcd = db['some_key']
#
# @endcode 
#
# Access existing DB in update mode :
#
# @code
#
# >>> import SQLiteShelve                   ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db' )      ## access existing dbase in update mode
# ...
# >>> for key in db : print key
# ...
# >>> abcd = db['some_key']
#
# @endcode 
#
# 
# @author Vanya BELYAEV Ivan.Belyaev@cern.ch
# @date   2010-04-30
# 
#                    $Revision$
#  Last Modification $Date$
#                 by $Author$
# =============================================================================
"""This is SQLite-version of shelve database.

Keeping the same interface and functionlity as shelve data base,
SQLiteShelve allows much more compact file size through
the compression of the content

The actual code has been inspired by <c>sqlitedict</c> ( see Google...)
The compression (with zlib) is added atop of original code 
 
Create new DB:

# >>> import SQLiteShelve                     ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db', 'n')    ## create new DB
# ...
# >>> abcde = ...
# >>> db['some_key'] =  abcde                 ## add information to DB
# ...
# >>> db.close()

Access to DB in read-only mode :

# >>> import SQLiteShelve                     ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db' , 'r' )  ## access existing dbase in read-only mode
# ...
# >>> for key in db : print key
# ...
# >>> abcd = db['some_key']


 Access existing DB in update mode :

# >>> import SQLiteShelve                    ## import the SQLiteShelve module 
# >>> db = SQLiteShelve.open ('a_db' )       ## access existing dbase in update mode
# ...
# >>> for key in db : print key
# ...
# >>> abcd = db['some_key']
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2014-06-19"
__version__ = "$Revision$" 
# =============================================================================
__all__ = (
    'SQLiteShelf' ,   ## The DB-itself
    'open'            ## helper function to hide the actual DB
    )
# =============================================================================
from AnalysisPython.Logger import getLogger
if '__main__' == __name__ : logger = getLogger ( 'SQLiteShelve' )
else                      : logger = getLogger ( __name__ )
# =============================================================================
from   Ostap.sqlitedict import SqliteDict
import zlib 
# =============================================================================
## @class SQLiteShelf
#  SQLite-based ``shelve-like'' database with compressed content. 
#  
class SQLiteShelf(SqliteDict):
    """
    """
    def __init__ ( self                     ,
                   filename       = None      ,
                   mode           = 'c'       ,
                   tablename      = 'PyPaw'   ,
                   writeback      = True      , ## original name: "autocommit"
                   compress_level = zlib.Z_BEST_COMPRESSION , 
                   journal_mode   = "DELETE"  ) :
        """
        Initialize a thread-safe sqlite-backed dictionary.
        The dictionary will be a table ``tablename`` in database file
        ``filename``. A single file (=database) may contain multiple tables.
        
        If no ``filename`` is given, a random file in temp will be used
        (and deleted from temp once the dict is closed/deleted).
        
        If you enable ``writeback/autocommit`` changes will be committed
        after each operation (more inefficient but safer).
        Otherwise, changes are committed on
        ``self.commit()``,
        ``self.clear()`` and
        ``self.close()``.
        
        Set ``journal_mode`` to ``OFF``
        if you're experiencing sqlite I/O problems
        or if you need performance and don't care about crash-consistency.
        
        The `flag` parameter:
        - 'c': default mode, open for read/write, creating the db/table if necessary.
        - 'w': open for r/w, but drop `tablename` contents first (start with empty table)
        - 'n': create a new database (erasing any existing tables, not just `tablename`!).
        
        """
        if not filename is None :
            import os 
            filename  = os.path.expandvars ( filename )
            filename  = os.path.expanduser ( filename )
            filename  = os.path.expandvars ( filename )
            filename  = os.path.abspath    ( filename )
            
        SqliteDict.__init__ ( self                        ,
                              filename     = filename     ,
                              tablename    = tablename    ,
                              flag         = mode         ,
                              autocommit   = writeback    ,
                              journal_mode = journal_mode )
        
        self.compression = compress_level 

    ## list the avilable keys 
    def __dir ( self , pattern = '' ) :
        """
        List the avilable keys (patterns included).
        Pattern matching is performed accoriding to
        fnmatch/glob/shell rules [it is not regex!] 
        
        >>> db = ...
        >>> db.ls() ## all keys
        >>> db.ls ('*MC*')
        
        """
        keys_ = self.keys()
        keys_.sort()
        if pattern :
            import fnmatch
            _keys = [ k for k in keys_ if fnmatch.fnmatchcase ( k , pattern ) ]
            keys_ = _keys
        #
        for key in keys_ : print key
        
    ## list the avilable keys 
    def ls    ( self , pattern = '' ) :
        """
        List the avilable keys (patterns included).
        Pattern matching is performed accoriding to
        fnmatch/glob/shell rules [it is not regex!] 

        >>> db = ...
        >>> db.ls() ## all keys
        >>> db.ls ('*MC*')        
        
        """
        return self.__dir( pattern )


# =============================================================================
try:
    from cPickle import Pickler, Unpickler, HIGHEST_PROTOCOL
except ImportError:
    from  pickle import Pickler, Unpickler, HIGHEST_PROTOCOL 

try:
    from cStringIO import StringIO
except ImportError:
    from  StringIO import StringIO
 
# =============================================================================
## ``get-and-uncompress-item'' from dbase 
def _zip_getitem (self, key):
    """
    ``get-and-uncompress-item'' from dbase 
    """
    GET_ITEM = 'SELECT value FROM %s WHERE key = ?' % self.tablename
    item = self.conn.select_one(GET_ITEM, (key,))
    if item is None: raise KeyError(key)
    
    f     = StringIO ( zlib.decompress ( str ( item[0] ) ) ) 
    value = Unpickler(f).load()

    return value

import sqlite3
# =============================================================================
## ``set-and-compress-item'' to dbase 
def _zip_setitem ( self , key , value ) :
    """
    ``set-and-compress-item'' to dbase 
    """
    ADD_ITEM = 'REPLACE INTO %s (key, value) VALUES (?,?)' % self.tablename
    
    f     = StringIO()
    p     = Pickler(f, HIGHEST_PROTOCOL  )
    p.dump(value)
    blob  = f.getvalue() 
    zblob = zlib.compress ( blob , self.compression ) 
    self.conn.execute(ADD_ITEM, (key, sqlite3.Binary( zblob ) ) )

                      
SQLiteShelf.__setitem__ = _zip_setitem
SQLiteShelf.__getitem__ = _zip_getitem

def _sql_enter_ ( self      ) : return self
def _sql_exit_  ( self , *_ ) :
    try :
        os.close()
    except : pass

SQLiteShelf.__enter__ = _sql_enter_
SQLiteShelf.__exit__  = _sql_exit_ 

# =============================================================================
## open new SQLiteShelve data base
def open(*args, **kwargs):
    """See documentation of the SQLiteShelf class."""
    return SQLiteShelf(*args, **kwargs)

# =============================================================================
if '__main__' == __name__ :

    import Ostap.Line
    logger.info ( __file__  + '\n' + Ostap.Line.line  ) 
    logger.info ( 80*'*'   )
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

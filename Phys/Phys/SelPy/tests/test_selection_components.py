#!/usr/bin/env python
#$Id: test_selection_components.py,v 1.2 2010/02/25 14:13:33 jpalac Exp $
'''
Test suite for selection module.
'''

__author__ = "Juan PALACIOS juan.palacios@nikhef.nl"

import sys
sys.path.append('../python')

from py.test import raises
from SelPy.configurabloids import MockConfGenerator
from SelPy.selection import (flatAlgorithmList,
                             Selection,
                             EventSelection,
                             PassThroughSelection,
                             AutomaticData,
                             NamedObject,
                             UniquelyNamedObject,
                             NameError)

class SelectionTree(object) :
    sel000 = Selection('0.00000', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [])
    sel001 = Selection('0.00001', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [])
    sel002 = Selection('0.00002', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [])
    sel003 = Selection('0.00003', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [])
    
    sel100 = Selection('1.00000', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel000, sel001])
    
    sel101 = Selection('1.00001', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel002, sel003])
    
    sel200 = Selection('2.00000', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel100, sel101])

    algos = flatAlgorithmList(sel200)
    alg_names = [a.name() for a in algos]


def test_NamedObject_name() :
    un = NamedObject('HelloWorld')
    assert un.name() == 'HelloWorld'

def test_uniquelyNamedObject_instantiaiton() :
    un = UniquelyNamedObject('Fred')

def test_UniquelyNamedObject_raises_on_duplicate_name() :
    un0 = UniquelyNamedObject('Bob')
    raises(NameError, UniquelyNamedObject, 'Bob')

def test_UniquelyNamedObject_name() :
    un = UniquelyNamedObject('HelloWorld')
    assert un.name() == 'HelloWorld'

def test_flatAlgorithmList_order_line() :

    
    sel000 = Selection('0.0000', ConfGenerator = MockConfGenerator(),
                      RequiredSelections = [])
    sel001 = Selection('0.0001', ConfGenerator = MockConfGenerator(),
                      RequiredSelections = [sel000])
    sel101 = Selection('1.0000', ConfGenerator = MockConfGenerator(),
                      RequiredSelections = [sel001])
    sel102 = Selection('1.0001', ConfGenerator = MockConfGenerator(),
                      RequiredSelections = [sel101])
    sel203 = Selection('2.0000', ConfGenerator = MockConfGenerator(),
                      RequiredSelections = [sel102])

    algos = flatAlgorithmList(sel203)
    assert [a.name() for a in algos] == ['0.0000', '0.0001', '1.0000', '1.0001', '2.0000']
    
def test_flatAlgorithmList_order_tree() :

    alg_names = SelectionTree.alg_names
    
    assert alg_names.index('0.00000') < alg_names.index('1.00000')
    assert alg_names.index('0.00001') < alg_names.index('1.00000')
    assert alg_names.index('0.00002') < alg_names.index('1.00001')
    assert alg_names.index('0.00003') < alg_names.index('1.00001')
    assert alg_names.index('1.00000') < alg_names.index('2.00000')
    assert alg_names.index('1.00001') < alg_names.index('2.00000')

def test_flatAlgorithmList_removes_duplicates() :
    
    sel000 = SelectionTree.sel000
    sel001 = SelectionTree.sel001
    sel002 = SelectionTree.sel002
    sel003 = SelectionTree.sel003

    sel100 = Selection('1.10000', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel000, sel001, sel002])
    
    sel101 = Selection('1.10001', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel001, sel002, sel003])
    
    sel200 = Selection('2.10000', ConfGenerator = MockConfGenerator(),
                       RequiredSelections = [sel100, sel101])

    algos = flatAlgorithmList(sel200)
    alg_names = [a.name() for a in algos]
    assert len(algos) == 7
    assert alg_names.count('0.00000') == 1
    assert alg_names.count('0.00001') == 1
    assert alg_names.count('0.00002') == 1
    assert alg_names.count('0.00003') == 1
    assert alg_names.count('1.10000') == 1
    assert alg_names.count('1.10001') == 1
    assert alg_names.count('2.10000') == 1
    
    assert alg_names.index('0.00000') < alg_names.index('1.10000')
    assert alg_names.index('0.00001') < alg_names.index('1.10000')
    assert alg_names.index('0.00002') < alg_names.index('1.10000')
    assert alg_names.index('0.00001') < alg_names.index('1.10001')
    assert alg_names.index('0.00002') < alg_names.index('1.10001')
    assert alg_names.index('0.00003') < alg_names.index('1.10001')
    assert alg_names.index('1.10000') < alg_names.index('2.10000')
    assert alg_names.index('1.10001') < alg_names.index('2.10000')
    
if '__main__' == __name__ :

    import sys

    test_names = filter(lambda k : k.count('test_') > 0, locals().keys())

    __tests = filter( lambda x : x[0] in test_names, locals().items())
    

    message = ''
    summary = '\n'
    length = len(sorted(test_names,
                        cmp = lambda x,y : cmp(len(y),len(x)))[0]) +2
    
    for test in __tests :
        try :
            test[1]()
            message = 'PASS'
        except :
            message = "FAIL"
        summary += test[0].ljust(length) + ':' + message.rjust(10) + '\n'

    if summary.count('FAIL') > 0 :
        message = 'FAIL'
        wr = sys.stderr.write
    else :
        message = 'PASS'
        wr = sys.stdout.write

    summary += 'Global'.ljust(length) + ':' + message.rjust(10) + '\n\n'
    wr(summary)
        

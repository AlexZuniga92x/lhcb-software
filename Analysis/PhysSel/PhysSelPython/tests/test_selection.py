#!/usr/bin/env python
#
# test Selection-like classes
#
import sys
from py.test import raises
sys.path.append('../python')
from PhysSelPython.Wrappers import Selection, AutomaticData, MergedSelection, NameError, NonEmptyInputLocations, IncompatibleInputLocations
from SelPy.configurabloids import DummyAlgorithm, DummySequencer

def test_automatic_data() :
    sel00 = AutomaticData(Location = 'Phys/Sel00x')
    assert sel00.name() == 'Sel00x'
    assert sel00.algName() == 'SelFilterSel00x'
    assert sel00.outputLocation() == 'Phys/Sel00x'

def test_automatic_data_does_not_accept_more_than_one_ctor_argument() :
    raises(TypeError, AutomaticData, 'name', Location = 'Phys/Sel00')

def test_automatic_data_with_no_location_raises() :
     raises(Exception, AutomaticData)

def test_merged_selection() :
    sel00 = AutomaticData(Location = 'Phys/Sel00')
    sel01 = AutomaticData(Location = 'Phys/Sel01')
    ms = MergedSelection('Merge00And01', RequiredSelections = [sel00, sel01])
    assert ms.name() == 'Merge00And01'
    assert ms.requiredSelections == [sel00, sel01]
    assert ms.outputLocation() == 'Phys/Merge00And01'
    assert [alg.name() for alg in ms.algos] == ['SelFilterSel01', 'SelFilterSel00', 'Merge00And01']
    assert ms.algos == [sel01.algorithm(), sel00.algorithm(), ms._sel.algorithm()]
    
def test_instantiate_tree(selID='0000') :
    sel00 = AutomaticData(Location = 'Phys/Sel00')
    sel01 = AutomaticData(Location = 'Phys/Sel01')
    alg0 = DummyAlgorithm('Alg000')
    sel0 = Selection(selID, Algorithm = alg0,
                     RequiredSelections = [sel00, sel01])
    return sel0

def test_outputLocation() :
    sel00 = AutomaticData(Location = 'Phys/Sel00')
    assert sel00.outputLocation() == 'Phys/Sel00'
    alg0 = DummyAlgorithm('Alg0001')
    sel0 = Selection('SomeName001', Algorithm = alg0)
    assert sel0.outputLocation() == 'Phys/SomeName001'
    sel1 = Selection('SomeName002', OutputBranch = 'HLT2', Algorithm = alg0)
    assert sel1.outputLocation() == 'HLT2/SomeName002'
    
def test_tree_InputLocations_propagated() :
    
    sel00 = AutomaticData(Location = 'Phys/Sel00')
    sel01 = AutomaticData(Location = 'Phys/Sel01')
    alg0 = DummyAlgorithm('Alg001')
    sel0 = Selection('Sel001', Algorithm = alg0,
                     RequiredSelections = [sel00, sel01])
    assert len(alg0.InputLocations) == 0
    assert len(sel0.alg.InputLocations) == 2
    assert sel0.alg.InputLocations.count('Phys/Sel00') ==1
    assert sel0.alg.InputLocations.count('Phys/Sel01') ==1


'''
#This requires the algorithm to raise. Gaudi algorithms do not seen to do this.
def test_selection_with_existing_algo_name_raises() :
    
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg003', InputLocations = ['Phys/Sel00', 'Phys/Sel01'])
    try :
        sel0 = Selection('Alg003', Algorithm = alg0,
                         RequiredSelections = [sel02, sel03])
        assert alg0.InputLocations == ['Phys/Sel00', 'Phys/Sel01']
    except NameError :
        print "NameError caught"
'''
def test_selection_with_existing_selection_name_raises() :
    
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg003')

    sel0 = Selection('Sel003', Algorithm = alg0,
                     RequiredSelections = [sel02, sel03])
    raises(NameError, Selection, 'Sel003', Algorithm = alg0,
                         RequiredSelections = [sel02, sel03])

    assert sel0.alg.InputLocations == ['Phys/Sel02', 'Phys/Sel03']

def test_clone_selection_with_existing_selection_name_raises() :
    
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg004')
    sel0 = Selection('Sel004', Algorithm = alg0,
                     RequiredSelections = [sel02, sel03])
    

    raises( NameError, sel0.clone, name = 'Sel004', Algorithm = alg0,
            RequiredSelections = [sel02, sel03])
    assert sel0.alg.InputLocations == ['Phys/Sel02', 'Phys/Sel03']
        
def test_clone_selection_with_new_alg() :
    sel0 = test_instantiate_tree('0001')
    alg1 = DummyAlgorithm('Alg1')
    selClone = sel0.clone('sel0_clone0', Algorithm = alg1)

    assert alg1.name() == 'Alg1'
    assert len(alg1.InputLocations) == 0

    assert len(sel0.alg.InputLocations) == 2
    assert sel0.alg.InputLocations.count('Phys/Sel00') ==1
    assert sel0.alg.InputLocations.count('Phys/Sel01') ==1

def test_clone_selection_with_new_InputLocations() :
    sel = test_instantiate_tree('0002')
    clone00 = AutomaticData(Location = 'Phys/Clone00')
    clone01 = AutomaticData(Location = 'Phys/Clone01')
    selClone = sel.clone('sel0_clone1', RequiredSelections = [clone00, clone01])
    assert len(selClone.alg.InputLocations) == 2
    assert selClone.alg.InputLocations.count('Phys/Clone00') ==1
    assert selClone.alg.InputLocations.count('Phys/Clone01') ==1

def test_selection_with_name_overlap_doesnt_raise() :
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg005')
    sel0 = Selection('Sel005', Algorithm = alg0)
    sel1 = Selection('Sel005Loose', Algorithm = alg0)
    assert sel0.outputLocation() == 'Phys/Sel005'
    assert sel1.outputLocation() == 'Phys/Sel005Loose'

def test_selection_with_different_InputLocations_set_in_algo_raises() :
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg006',
                          InputLocations = ['Phys/Sel00', 'Phys/Sel01'])
    raises(IncompatibleInputLocations, Selection, 'Sel006', Algorithm = alg0,
           RequiredSelections = [sel02, sel03])

def test_selection_with_same_InputLocations_set_in_algo() :
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    alg0 = DummyAlgorithm('Alg007',
                          InputLocations = ['Phys/Sel02', 'Phys/Sel03'])
    sel0 = Selection('Sel007', Algorithm = alg0,
                     RequiredSelections = [sel02, sel03])
        
def test_clone_selection_with_cloned_alg() :
    pass

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
        

#!/usr/bin/env python

# first parse all options
from optparse import OptionParser
parser = OptionParser(usage = "%prog [options] <opts_file> ...")
parser.add_option("-n","--numiter",type="int", dest="numiter",help="number of iterations", default=3)
parser.add_option("-f","--firstiter",type="int", dest="firstiter",help="first iterations", default=0)
parser.add_option("-e","--numevents",type="int", dest="numevents",help="number of events", default=1000)
parser.add_option("-p","--numprocesses",type="int", dest="numprocs",help="number of processes", default=8)
parser.add_option("-d", "--aligndb", action = 'append', dest="aligndb",help="path to file with alignment database layer for first iteration")
parser.add_option("-r", "--roothistofile",dest="histofile",help="name of histogram file",default = "histograms.root")
(opts, args) = parser.parse_args()

import os
rundir = os.getcwd()

for i in range(opts.firstiter,opts.numiter) :
    print "Iteration nr: ", i, "\n"
    # make the directory from where to run the job
    iterdir = 'Iter' + str( i )
    if os.path.isdir( iterdir ) :
        print "Directory exists. Will skip this iteration."
        continue
    
    os.mkdir ( iterdir )
    os.chdir ( iterdir )

    # beyond the first iteration, add the input database as an option
    theseoptions = ' --numevents ' + str(opts.numevents) + ' --numprocesses ' + str(opts.numprocs) + ' --iter ' + str(i) + ' -r ' + opts.histofile + ' '
    if i>0 :
        previterdb = '../Iter' + str( i-1 ) + '/Alignment.db'
        theseoptions += ' --aligndb ' + previterdb
    else :
        if opts.aligndb :
            for db in opts.aligndb :
                theseoptions += ' -d ' + db

    # add the remaining options
    for a in args:
        theseoptions += ' ' + a
        os.system( ' cp -f ' + a + ' .' )

    # run the job
    thiscommand = '$ESCHEROPTS/gaudipar.py' + theseoptions + '>& logfile.txt'
    print 'command: ', thiscommand
    os.system( thiscommand )
    os.system( 'gzip logfile.txt' )
    # keep only the last version of the derivatives. they take too much space.
    # os.system( 'mv -f myderivatives.dat ..')
    os.chdir(rundir)

# create a single alignlog file
os.system('rm -f alignlog.txt')
os.system('cat Iter?/alignlog.txt Iter1?/alignlog.txt Iter2?/alignlog.txt > alignlog.txt')

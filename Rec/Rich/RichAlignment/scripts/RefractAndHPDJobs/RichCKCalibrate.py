
# Globals
imageFileName = ''
canvas        = None
runInfoCacheName = "RunInfoCache.pck.bz2"
runInfoCacheLoaded = False
runInfoCache  = { }
#histoBase = 'RICH/RiCKResLong/'
histoBase = 'RICH/RiCKResLongTight/'

# ====================================================================================
# Main Methods
# ====================================================================================

## Submits Control Jobs
def submitControlJobs(name="",pickedRuns="Run71813-LFNs.pck.bz2"):
    
    import os
    from Ganga.GPI import ( Job, LHCbDataset, Brunel, File, DiracSplitter, 
                            SmartMerger, Dirac )
    
    # Number of target events to process
    nEventsTotal    = 250000
    nFilesMax       = 10000

    RunLFNs = getRunLFNData(pickedRuns)

    # Loop over runs
    for run in sorted(RunLFNs.keys()):

        # LFNs for this run
        lfns = RunLFNs[run]
        if len(lfns)>0 :

            nFiles = len(lfns)
            if nFiles > nFilesMax : nFiles = nFilesMax
            nFilesPerJob = filesPerJob(nFiles)
            nEventsPerJob = nFilesPerJob * nEventsTotal / nFiles
            print "Using", nFiles, "data file(s),", nEventsPerJob, "events per job"

            r = drange( 0.98000, 1.010, 0.001 )
            R1N = [ '%g' % x for x in r ]
    
            r = drange( 1.055, 1.085, 0.001 )
            R2N = [ '%g' % x for x in r ]
        
            for i in range(0,len(R1N)):

                r1 = R1N[i]
                r2 = R2N[i]

                print "(n-1) Scale Rich1 =",r1,"Rich2",r2
            
                # Make a job object
                j = Job( application = Brunel( version = 'v40r1' ) )

                # name
                j.name = "RefInControl"
                if name != "" : j.name += "-"+name
                j.name += "_BR-"+BrunelVer
                j.name += "_Run-"+str(run)+"_R1-"+r1+"_R2-"+r2
                print "Submitting Job", j.name

                # Custom options for this job
                tmpOptsFile = createTempOptsFile(j.name)
                extraopts = open(tmpOptsFile,"w")
                extraopts.write("from Configurables import Brunel\n")
                extraopts.write("from Brunel.Configuration import *\n")
                extraopts.write("from Configurables import UpdateManagerSvc\n")
                extraopts.write("ums = UpdateManagerSvc()\n")
                extraopts.write("ums.ConditionsOverride += [\"Conditions/Environment/Rich1/RefractivityScaleFactor := double CurrentScaleFactor = "+r1+";\"]\n")
                extraopts.write("ums.ConditionsOverride += [\"Conditions/Environment/Rich2/RefractivityScaleFactor := double CurrentScaleFactor = "+r2+";\"]\n")
                extraopts.write("HistogramPersistencySvc().OutputFile = \""+j.name+".root\"\n")
                extraopts.write("Brunel().EvtMax = "+str(nEventsPerJob)+"\n")
                extraopts.close()

                # Job options
                j.application.optsfile = [ File('CKRefractCalib-FullBrunelReco.py'),
                                           File(tmpOptsFile) ]

                # Set the LFNs to run over
                j.inputdata = LHCbDataset(lfns)

                # Split job into 1 file per subjob
                j.splitter = DiracSplitter ( filesPerJob = nFilesPerJob, maxFiles = nFiles )

                # Merge the output
                j.merger = SmartMerger( files = [j.name+".root"],
                                        ignorefailed = True, overwrite = True )

                # Optional input files
                j.inputsandbox = []

                # Dirac backend
                j.backend = Dirac()

                # Enable automatic job re-submission
                j.do_auto_resubmit = True

                # Submit !!
                j.submit()

## Submits DB calibration jobs
def submitCalibrationJobs(name="",BrunelVer="v40r1",pickledRunsList=[]):
    submitRecoJobs(name,BrunelVer,pickledRunsList,"RefInCalib")

## Submit DB Verification Jobs
def submitVerificationJobs(name="",BrunelVer="v40r1",pickledRunsList=[]):
    submitRecoJobs(name,BrunelVer,pickledRunsList,"RefInVerify")

## Real underlying method
def submitRecoJobs(name,BrunelVer,pickledRunsList,jobType):

    from Ganga.GPI import ( Job, LHCbDataset, Brunel, File,
                            DiracSplitter, SmartMerger, Dirac )

    # If pickled run data list is empty, create full list
    if len(pickledRunsList) == 0 : pickledRunsList = getPickledRunList()

    # Number of target events to process
    if jobType == "RefInCalib" :
        nEventsTotal    = 100000
        nFilesMax       = 100
    else:
        nEventsTotal    = 100000
        nFilesMax       = 100

    # Base Job Name
    basejobname = jobType
    if name != "" : basejobname += "-"+name
    basejobname += "_BR-"+BrunelVer

    # My input sandboxs
    mySandBox     = [ ]
    mySandBoxLFNs = [ ]

    # List of DB related options to add to extraopts
    dbopts = [ ]

    # Main LHCbCond
    #Cond = "LHCBCOND-2001RePro.db"
    #dbopts += ["CondDB().PartitionConnectionString[\"LHCBCOND\"] = \"sqlite_file:"+Cond+"/LHCBCOND\"\n"]
    ##dbopts += ["LHCbApp().CondDBtag = \"HEAD\"\n"]
    #lfnname = "LFN:/lhcb/user/j/jonrob/DBs/"+Cond
    #uploadFile("databases/"+Cond,lfnname)
    #mySandBoxLFNs += [lfnname]
    
    # Main DDDB
    #Cond = "DDDB-2001RePro.db"
    #dbopts += ["CondDB().PartitionConnectionString[\"DDDB\"] = \"sqlite_file:"+Cond+"/DDDB\"\n"]
    ##dbopts += ["LHCbApp().DDDBtag = \"HEAD\"\n"]
    #lfnname = "LFN:/lhcb/user/j/jonrob/DBs/"+Cond
    #uploadFile("databases/"+Cond,lfnname)
    #mySandBoxLFNs += [lfnname]

    # DDDB Slice
    #Cond = "JulyAugustOnly-OT-DDDB.db"
    #dbopts += ["CondDB().addLayer(dbFile = \""+Cond+"\", dbName = \"DDDB\")\n"]
    ###dbopts += ["LHCbApp().DDDBtag = \"HEAD\"\n"]
    #lfnname = "LFN:/lhcb/user/j/jonrob/DBs/"+Cond
    #uploadFile("databases/"+Cond,lfnname)
    #mySandBoxLFNs += [lfnname]

    # Custom DB slices for both job types (calibration and verification)
    dbFiles = [ ]

    # Corrections RICH
    dbFiles += ["2011MirrorAlign-27102011"]
    dbFiles += ["2011-PhaseTwoRePro-RootFiles-RunAligned-Sobel-Smoothed0.5hours-HPDOcc-21102011"]
    dbFiles += ["2011-PhaseTwoRePro-RootFiles-RunAligned-Sobel-Smoothed1.5hours-HPDAlign-21102011"]

    # Tracking
    #dbFiles += ["TrackingDB-v5.6series","Velo2011"] 

    # Only for Calibration jobs only
    if jobType == "RefInCalib" :
        dbopts += ["UpdateManagerSvc().ConditionsOverride += [\"Conditions/Environment/Rich1/RefractivityScaleFactor := double CurrentScaleFactor = 1.0;\"]\n"]
        dbopts += ["UpdateManagerSvc().ConditionsOverride += [\"Conditions/Environment/Rich2/RefractivityScaleFactor := double CurrentScaleFactor = 1.0;\"]\n"]

    # For verification jobs only, use custom DB Slice for n-1 corrections
    if jobType == "RefInVerify" :
        #pass
        dbFiles += ["RefInCalib-2011-NewTkRichAlign-V1_BR-v40r1-21102011"]

    # Configure additional DBs
    for dbFile in dbFiles :
        dbopts += ["CondDB().addLayer(CondDBAccessSvc(\""+dbFile+"\",ConnectionString=\"sqlite_file:"+dbFile+".db/LHCBCOND\",DefaultTAG=\"HEAD\"))\n"]
        # Add to actual sandbox
        #mySandBox += ["databases/"+dbFile+".db"]
        # Upload to LFNs
        lfnname = "LFN:/lhcb/user/j/jonrob/DBs/"+dbFile+".db"
        if not uploadFile("databases/"+dbFile+".db",lfnname) : return False
        # Add to LFNs sandbox
        mySandBoxLFNs += [lfnname]

    # Loop over the list of pickled run data files
    print "Submitting jobs for RunData", pickledRunsList
    for pickedRuns in pickledRunsList :

        # Loop over runs
        print "Reading runs from", pickedRuns
        RunLFNs = getRunLFNData(pickedRuns)
        sortedRuns = sorted(RunLFNs.keys())
        
        print "Submitting", jobType, "jobs for runs", sortedRuns
        nJob = 0
        for run in sortedRuns:

            # Count jobs
            nJob += 1

            # LFNs for this run
            lfns = sorted(RunLFNs[run])
            if len(lfns)>0 :

                # Construct the job name
                jobname = basejobname + "_Run-"+str(run)

                # is this job already submitted ?
                if jobExists(jobname):

                    print "Job", jobname, "already submitted ( #", nJob, "of", len(sortedRuns), ")"

                else:

                    # Configure number of files and events per file
                    nFiles = len(lfns)
                    if nFiles > nFilesMax : nFiles = nFilesMax
                    nFilesPerJob = filesPerJob(nFiles)
                    nEventsPerJob = nFilesPerJob * nEventsTotal / nFiles
          
                    # Make a job object
                    j = Job( application = Brunel( version = BrunelVer ), name = jobname )

                    # Custom options for this job
                    tmpOptsFile = createTempOptsFile(j.name)
                    extraopts = open(tmpOptsFile,"w")

                    # Basic additions
                    extraopts.write("from Brunel.Configuration import *\n")
                    extraopts.write("from Gaudi.Configuration import *\n")
                    extraopts.write("from Configurables import Brunel, UpdateManagerSvc, CondDB, CondDBAccessSvc, LHCbApp\n")
                    extraopts.write("HistogramPersistencySvc().OutputFile = \""+j.name+".root\"\n")
                    extraopts.write("Brunel().EvtMax = "+str(nEventsPerJob)+"\n")

                    # Add DB options
                    for dbopt in dbopts : extraopts.write(dbopt)

                    # Close file
                    extraopts.close()

                    # Job options
                    j.application.optsfile = [ File('CKRefractCalib-FullBrunelReco.py'),
                                               File(extraopts.name) ]

                    # Set the LFNs to run over
                    if nFiles == len(lfns):
                        j.inputdata = LHCbDataset(lfns)
                    else:
                        import random
                        j.inputdata = LHCbDataset(sorted(random.sample(lfns,nFiles)))

                    # Split job into 1 file per subjob
                    j.splitter = DiracSplitter ( filesPerJob = nFilesPerJob, maxFiles = nFiles )

                    # Merge the output
                    j.merger = SmartMerger( files = [j.name+".root"],
                                            ignorefailed = True, overwrite = True )

                    # Dirac backend
                    j.backend = Dirac()
                    # j.backend = Dirac( settings = {'CPUTime':50000} )
                    
                    # Force jobs to go to CERN only
                    #j.backend.settings['Destination'] = 'LCG.CERN.ch'

                    # Optional input files
                    j.inputsandbox             = mySandBox
                    j.backend.inputSandboxLFNs = mySandBoxLFNs

                    # Enable automatic job re-submission
                    j.do_auto_resubmit = True

                    # Add to jobtree
                    addToJobTree(j,basejobname)

                    # Submit !!
                    print "Submitting Job", j.name, "( #", nJob, "of", len(sortedRuns), ")"
                    print " -> Using", nFiles, "data file(s), max", nFilesPerJob, \
                          "file(s) per subjob,", nEventsPerJob, "events per job"

                    for f in j.inputdata.files : print "  ->", f.name
                    try:
                        j.submit()
                    except Exception,e:
                        print "WARNING : Job not submitted"
                        j.remove()          

def refractiveIndexCalib(jobs,rad='Rich1Gas'):

    from Ganga.GPI import Job
    from ROOT import TFile, TGraphErrors, TF1, TH1, TH1F, gROOT
    from array import array
    import pickle, bz2

    if len(jobs) == 0 : return

    # File name root
    fileNameRoot = rad+"_"+getJobCaliName(jobs[0])

    # Start a PDF file
    globals()["imageFileName"] = "results/"+fileNameRoot+".pdf"
    printCanvas('[')

    # Dictionary to store the calibration data
    calibrations = { }

    # Keep tabs on min and max values (for plots)
    minMaxScale = [999.0,-999.0]
    if 'Rich1Gas' == rad :
        minMaxCKRes = (0.0014,0.00185)
        maxDeltaTheta = 0.00003
    else:
        minMaxCKRes = (0.00063,0.00075)
        maxDeltaTheta = 0.00001

    # Raw mean and sigma
    ckmeans  = { }
    cksigmas = { }
    ckraws   = { }
    ckexpect = { }

    # Max/min run range
    #minMaxRun = [ 0, 99999999 ]
    minMaxRun = [ 87657, 99999999 ] # Skip first runs of 2011 with bad gas mixtures
    #minMaxRun = [ 101372, 99999999 ] # Second phase of 2011 RePro

    # Bad runs to always skip
    badRuns = [ 89537 ]

    # Loop over jobs
    nFailedFits = 0
    print "Looping over the runs ..."
    for j in jobs :

        # Run Number
        run = int(getInfoFromJob(j,'Run'))
        if run >= minMaxRun[0] and run <= minMaxRun[1] and run not in badRuns :

            # Root file
            rootfile = getRootFile(j)

            # Fits
            fitResultRes = fitCKThetaHistogram(rootfile,run,rad)
            fitResultRaw = fitCKThetaHistogram(rootfile,run,rad,'thetaRec',-1)
            fitResultExp = fitCKExpectedHistogram(rootfile,run,rad)
        
            if fitResultRes['OK'] and fitResultRaw['OK'] and fitResultExp['OK'] :
                scale = nScaleFromShift(fitResultRes,rad)
                if scale[0] < minMaxScale[0] : minMaxScale[0] = scale[0]
                if scale[0] > minMaxScale[1] : minMaxScale[1] = scale[0]
                calibrations[run] = { "ScaleFactor" : scale, "ThetaShift" : fitResultRes['Mean'] }
                ckmeans[run]  = fitResultRes['Mean']
                cksigmas[run] = fitResultRes['Sigma']
                ckraws[run]   = fitResultRaw['Mean']
                ckexpect[run] = fitResultExp['Mean']
            else:
                nFailedFits += 1
                print "WARNING : fits failed for run", run
                print "        : CK resolution :", fitResultRes['Message']
                print "        : CK theta      :", fitResultRaw['Message']
                print "        : CK expected   :", fitResultExp['Message']

            # Close the root file
            rootfile.Close()

        else:
            print " -> Skipping run", run

    # Write out calibrations to a pickled python file
    calibfilename = "results/"+rad+"_"+getJobCaliName(jobs[0])+".pck.bz2"
    print "Writting calibrations to", calibfilename
    file = bz2.BZ2File(calibfilename,"w")
    pickle.dump(calibrations,file)
    file.close()

    # 1D Plot of scale factors
    scaleHist = TH1F( "scaleFactors", rad+" (n-1) Scale Factors",
                      100, 0.999*minMaxScale[0], 1.0001*minMaxScale[1] )
    scaleHist.GetXaxis().SetTitle("(n-1) corrections")

    # 1D Plot of theta shifts
    dThetaHist = TH1F( "deltaTheta", rad+" <Delta CK Theta>",
                       100, -1.001*maxDeltaTheta, 1.001*maxDeltaTheta )
    dThetaHist.GetXaxis().SetTitle("<Delta CK Theta> / mrad")

    # 1D Plot of Fitted CK resolutions
    ckResHist = TH1F( "ckRes", rad+" Delta CK Theta Resolution",
                      100, 0.99*minMaxCKRes[0], 1.001*minMaxCKRes[1] )
    ckResHist.GetXaxis().SetTitle("Delta CK Theta Resolution / mrad")

    # Open text file for shifts
    textFileName = "results/"+fileNameRoot+".txt"
    print "Opening text file", textFileName
    textShifts = open(textFileName,'w')
  
    # For plots (manually make sure sorted by run)
    runs      = array('d')
    runsErr   = array('d')
    scales    = array('d')
    scalesErr = array('d')
    means     = array('d')
    meansErr  = array('d')
    sigmas    = array('d')
    sigmasErr = array('d')
    ckraw     = array('d')
    ckrawErr  = array('d')
    ckexp     = array('d')
    ckexpErr  = array('d')
    for run in sorted(calibrations.keys()):
        scale   = calibrations[run]["ScaleFactor"]
        ckmean  = ckmeans[run]
        cksigma = cksigmas[run]
        raw     = ckraws[run]
        exp     = ckexpect[run]
        # Write to text file
        writeInfoToTextFile(textShifts,run,raw,exp,ckmean,cksigma,scale)
        # plots
        runs.append(float(run))
        runsErr.append(0.0)
        scales.append(scale[0])
        scalesErr.append(scale[1])
        means.append(ckmean[0])
        meansErr.append(ckmean[1])
        sigmas.append(cksigma[0])
        sigmasErr.append(cksigma[1])
        ckraw.append(raw[0])
        ckrawErr.append(raw[1])
        ckexp.append(exp[0])
        ckexpErr.append(exp[1])
        # Fill 1D histo(s)
        scaleHist.Fill(scale[0])
        ckResHist.Fill(cksigma[0])
        dThetaHist.Fill(ckmean[0])

    # Make the plots
    if len(runs) > 0 :

        linearFit = TF1("AverageFit","pol0",runs[0],runs[len(runs)-1])
        linearFit.SetParName(0,"Mean")

        ckrawTrend = TGraphErrors( len(runs),runs,ckraw,runsErr,ckrawErr )
        ckrawTrend.SetTitle( rad+" Peak CK Theta by Run" )
        ckrawTrend.GetXaxis().SetTitle("LHCb Run Number")
        ckrawTrend.GetYaxis().SetTitle("Peak CK Theta / mrad")
        ckrawTrend.Draw("ALP")
        printCanvas()

        ckexpTrend = TGraphErrors( len(runs),runs,ckexp,runsErr,ckexpErr )
        ckexpTrend.SetTitle( rad+" <Expected CK Theta> by Run" )
        ckexpTrend.GetXaxis().SetTitle("LHCb Run Number")
        ckexpTrend.GetYaxis().SetTitle("<Expected CK Theta> / mrad")
        ckexpTrend.Draw("ALP")
        printCanvas()

        meanFitFunc = TF1("CKMean"+rad,"gaus",-maxDeltaTheta,maxDeltaTheta)
        dThetaHist.Fit(meanFitFunc,"QR")
        dThetaHist.Draw('E')
        printCanvas()
            
        meanTrend = TGraphErrors( len(runs),runs,means,runsErr,meansErr )
        meanTrend.SetTitle( rad+" <Delta CK Theta> by Run" )
        meanTrend.GetXaxis().SetTitle("LHCb Run Number")
        meanTrend.GetYaxis().SetTitle("<Delta CK Theta> / mrad")
        meanTrend.Draw("ALP")
        printCanvas()

        ckFitFunc = TF1("CKRes"+rad,"gaus",minMaxCKRes[0],minMaxCKRes[1])
        ckResHist.Fit(ckFitFunc,"QR")
        ckResHist.Draw('E')
        printCanvas()

        sigmaTrend = TGraphErrors( len(runs),runs,sigmas,runsErr,sigmasErr )
        sigmaTrend.SetTitle( rad+" Delta CK Theta Resolution by Run" )
        sigmaTrend.GetXaxis().SetTitle("LHCb Run Number")
        sigmaTrend.GetYaxis().SetTitle("Delta CK Theta Resolution / mrad")
        sigmaTrend.Fit(linearFit,"QRS")
        sigmaTrend.Draw("ALP")
        printCanvas()

        fitFunc = TF1("Scale"+rad,"gaus",minMaxScale[0],minMaxScale[1])
        scaleHist.Fit(fitFunc,"QR")
        scaleHist.Draw('E')
        printCanvas()

        scaleTrend = TGraphErrors( len(runs),runs,scales,runsErr,scalesErr )
        scaleTrend.SetTitle( rad+" (n-1) corrections by Run" )
        scaleTrend.GetXaxis().SetTitle("LHCb Run Number")
        scaleTrend.GetYaxis().SetTitle("(n-1) Scale Factor")
        #scaleTrend.Fit(linearFit,"QRS")
        scaleTrend.Draw("ALP")
        printCanvas()
       
    # Close PDF file
    printCanvas(']')

    if nFailedFits > 0 :
        print "WARNING :", nFailedFits, "histogram fits failed"

def writeInfoToTextFile(file,run,ckraw,ckexp,ckmean,cksigma,scale):
    runInfo = getRunInformation(run)
    fill = runInfo['FillNumber']
    desc = runInfo['DataTakingDescription']
    text = ( str(run) + " " + str(fill) + " " + desc
             + " " + str(runInfo['RunStart']) + " " + str(runInfo['RunEnd'])
             + " " + str(ckraw[0])   + " " + str(ckraw[1])
             + " " + str(ckexp[0])   + " " + str(ckexp[1])
             + " " + str(ckmean[0])  + " " + str(ckmean[1])
             + " " + str(cksigma[0]) + " " + str(cksigma[1])
             + " " + str(scale[0])   + " " + str(scale[1]) )
    file.write(text+"\n")
    
def refractiveIndexControl(jobs,rad='Rich1Gas'):
    
    from Ganga.GPI import Job
    from ROOT import TGraphErrors
    from array import array

    # Start a PDF file
    globals()["imageFileName"] = "results/"+rad+"-RefIndexControl.pdf"
    printCanvas('[')

    x  = array('d')
    xe = array('d')
    y  = array('d')
    ye = array('d')

    indexname = "R1"
    if rad == "Rich2Gas" : indexname = "R2"

    for j in jobs :

        run      = int(getInfoFromJob(j,'Run'))
        refIndex = getInfoFromJob(j,indexname)
        rootfile = getRootFile(j)
  
        print "Fitting job", rad, "Run =", run, "index =", refIndex
        fitResult = fitCKThetaHistogram(rootfile,run,rad)

        if fitResult['OK'] :
            x.append(float(refIndex))
            xe.append(0.0)
            y.append(float(fitResult['Mean'][0]))
            ye.append(float(fitResult['Mean'][1]))
        else:
            print " -> Fit failed -", fitResult['Message']

        rootfile.Close()

    graph = TGraphErrors( len(x),x,y,xe,ye )
    graph.SetTitle( rad+" (n-1) Control plot" )
    graph.GetXaxis().SetTitle("(n-1) Scale factor")
    graph.GetYaxis().SetTitle("CK theta shift")
    graph.Draw("ALP")
    printCanvas()
    
    maxIndex = len(x) - 1
    deltax = x[maxIndex] - x[0]
    deltay = y[0] - y[maxIndex]

    slope = deltax/deltay
    print "(n-1) correction =", slope, "* observed peak shift"
    
    # Close PDF file
    printCanvas(']')

def expectedCKTheta(jobs,rad='Rich1Gas'):

    from Ganga.GPI import Job
    from ROOT import TGraphErrors, TH1
    from array import array

    # Parameters
    minEntries = 10000

    globals()["imageFileName"] = "results/"+rad+"-ExpectCKtheta.pdf"
    printCanvas('[')

    runs      = array('d')
    runsErr   = array('d')
    exp       = array('d')
    expErr    = array('d')

    for j in jobs :

        # Run number
        run = int(getInfoFromJob(j,'Run'))

        # Root file
        rootfile = getRootFile(j)

        # get info from plot
        result = fitCKExpectedHistogram(rootfile,run,rad)
        if result['OK'] :

            # Mean of the histo
            mean = result['Mean'][0]
            err  = result['Mean'][1]
            
            runs.append(float(run))
            runsErr.append(0.0)
            exp.append(mean)
            expErr.append(err)
    
        rootfile.Close()
                
    # Make a plot
    graph = TGraphErrors( len(runs),runs,exp,runsErr,expErr )
    graph.SetTitle( rad+" <Expected CK theta> by Run" )
    graph.GetXaxis().SetTitle("Run Number")
    graph.GetYaxis().SetTitle("<Expected CK theta>")
    graph.Draw("ALP")
    printCanvas()
       
    # Close output file
    printCanvas(']')

def recoCKTheta(jobs,rad='Rich1Gas'):

    from Ganga.GPI import Job
    from ROOT import TGraphErrors
    from array import array
    import pickle

    # Start a PDF file
    globals()["imageFileName"] = "results/"+rad+"-RecoCKtheta.pdf"
    printCanvas('[')

    runs      = array('d')
    runsErr   = array('d')
    reco      = array('d')
    recoErr   = array('d')

    for j in jobs :

        run      = int(getInfoFromJob(j,'Run'))
        rootfile = getRootFile(j)

        fitResult = fitCKThetaHistogram(rootfile,run,rad,'thetaRec')

        if fitResult['OK'] :
            runs.append(float(run))
            runsErr.append(0.0)
            reco.append(float(fitResult['Mean'][0]))
            recoErr.append(float(fitResult['Mean'][1]))
        else:
            print " -> Fit failed -", fitResult['Message']

        rootfile.Close()
            
    # Make a plot
    graph = TGraphErrors( len(runs),runs,reco,runsErr,recoErr )
    graph.SetTitle( rad+" Peak Reco CK Theta by Run" )
    graph.GetXaxis().SetTitle("Run Number")
    graph.GetYaxis().SetTitle("CK Theta Peak / mrad")
    graph.Draw("ALP")
    printCanvas()
       
    # Close PDF file
    printCanvas(']')

# ====================================================================================
# Utility Methods
# ====================================================================================

def uploadFile(pfn,lfn,sites=['CERN-USER','RAL-USER','IN2P3-USER',
                              'PIC-USER','CNAF-USER','NIKHEF-USER','GRIDKA-USER']):
    
    from Ganga.GPI import PhysicalFile, LogicalFile
    import time

    if len(sites) == 0 :
        print 'ERROR : No sites requested'
        return False

    # Check if file has any replicas to start with
    res = LogicalFile(lfn).getReplicas()
    OK = True
    if len(res) < len(sites) :

        print "Uploading", pfn, "to", sites[0], lfn

        # First upload one copy
        if len(res) == 0 :
            newlfn = PhysicalFile(pfn).upload(lfn,sites[0])
            time.sleep(30)
            if len(newlfn.getReplicas()) == 0:
                print "Problem uploading ..."
                OK = False

        if OK :
            print " -> Upload SUCCESSFUL"
            # Replicate to requested sites
            for site in sites :
                try:
                    print '  -> Replicating to', site
                    LogicalFile(lfn).replicate(site)
                except Exception,e:
                    print '   -> ERROR Replication FAILED'
                    
    else:
        print lfn, "already exists as", res

    return OK

def uploadDBs(dbFiles=[]):
    for dbFile in dbFiles :
        lfnname = "LFN:/lhcb/user/j/jonrob/DBs/"+dbFile
        if not uploadFile("databases/"+dbFile,lfnname) : return False
    return True

def addToJobTree(j,dir):
    from Ganga.GPI import jobtree
    fulldir = "/RichCalibration/"+dir
    if not jobtree.exists(fulldir) : jobtree.mkdir(fulldir)
    jobtree.cd(fulldir)
    jobtree.add(j,fulldir)
    
def jobExists(jobname):
    from Ganga.GPI import jobs
    slice = jobs.select(name=jobname)
    return len(slice) > 0

def getInfoFromJob(j,info='Run'):
    run = 0
    split = ''
    splits = j.name.split('_')
    for split in splits:
        s = split.split('-')
        if s[0] == info : run = s[1]
    return run

def getJobCaliName(j):
    splits = j.name.split('_')
    cName = splits[0]
    if len(splits) == 3 : cName += "_"+splits[1]
    return cName

def loadDict(filename):
    import pickle, os, bz2
    data = { }
    if os.path.exists(filename) :
        file = bz2.BZ2File(filename,"r")
        data = pickle.load(file)
        file.close()
    return data

def pickleDict(filename,data):
    import pickle, os, bz2
    file = bz2.BZ2File(filename,"w")
    pickle.dump(data,file)
    file.close()

def loadRunInfoCache():
    if not globals()["runInfoCacheLoaded"]:
        cachename = globals()["runInfoCacheName"]
        print "Loading Run Info cache -", cachename
        globals()["runInfoCache"] = loadDict(cachename)
        globals()["runInfoCacheLoaded"] = True
    return globals()["runInfoCache"]

def saveRunInfoCache():
    cachename = globals()["runInfoCacheName"]
    #print "Saving Run Info cache  -", cachename
    pickleDict(cachename,globals()["runInfoCache"])

def queryBKDB(run):
    
    print "Getting information for run", run, "from BK API... Be patient..."
    
    from Ganga.GPI import diracAPI
    import time
    
    cmd = ( "from LHCbDIRAC.NewBookkeepingSystem.Client.BookkeepingClient import BookkeepingClient;" +
            "result = BookkeepingClient().getRunInformations("+str(run)+")" )

    res = { 'OK' : False }
    nTries = 0
    while not res['OK'] and nTries < 10:
        nTries = nTries + 1
        
        # Get information from Dirac
        res = diracAPI(cmd)
        print res

        if not res['OK'] :
            print " -> Problem querying DB - Will try again after 5 secs ..."
            time.sleep(5)

    if res['OK'] :
        
        # cached info
        runInfoCache = loadRunInfoCache()
        # Save
        saveBKQuery(run,res,runInfoCache)
        
    # return result
    return res

def saveBKQuery(run,res,runInfoCache):
    if len(res.keys()) > 0 and res.has_key('OK'):
        if res['OK']:
            runInfoCache[run] = res
            # Save the cache file
            saveRunInfoCache()

def getRunInformation(run):
    
    # cached info
    runInfoCache = loadRunInfoCache()

    # If cache has entry for this run use that. Otherwise fill.
    if run in runInfoCache.keys():
        res = runInfoCache[run]
    else:
        res = queryBKDB(run)
 
    # Checks
    if not isinstance(res,dict):
        print "ERROR : DB query result is not a dictionary... Retry..."
        res = queryBKDB(run)
    if not res.has_key('Value'):
        print "ERROR : DB query result has no key 'Value'... Retry..."
        res = queryBKDB(run)
    if not res.has_key('OK') or not res['OK']:
        print "ERROR : DB query result is not OK... Retry..."
        res = queryBKDB(run)
    if not isinstance(res['Value'],dict):
        print "ERROR : DB query result['Value'] is not a dictionary... Retry..."
        res = queryBKDB(run)

    # return the result
    info = { }
    if res['OK'] : info = res['Value']
    return info
                        
def getListOfJobs(tag,name,BrunelVer,statuscodes,MinRun=0,MaxRun=99999999,desc=""):
    from Ganga.GPI import Job, jobs
    cJobs = [ ]
    dict = { }
    searchString = tag
    if name != "" : searchString += "-"+name
    for j in jobs :
        if j.status in statuscodes :
            namesplit = j.name.split('_')
            if namesplit[0] == searchString :
                if BrunelVer == '' or namesplit[1] == "BR-"+BrunelVer :
                    run = int(getInfoFromJob(j,'Run'))
                    if run >= MinRun and run <= MaxRun:
                        if desc == "":
                            dict[run] = j
                        else:
                            runInfo = getRunInformation(run)
                            if runInfo['DataTakingDescription'] == desc:
                                dict[run] = j
    for d in sorted(dict.keys()) : cJobs += [dict[d]]
    return cJobs

def getCalibrationJobList(name="",BrunelVer="v40r1",statuscodes=['completed'],
                          MinRun=0,MaxRun=99999999,desc=""):
    return getListOfJobs('RefInCalib',name,BrunelVer,statuscodes,MinRun,MaxRun,desc)

def getVerificationJobList(name="",BrunelVer="v40r1",statuscodes=['completed'],
                           MinRun=0,MaxRun=99999999,desc=""):
    return getListOfJobs('RefInVerify',name,BrunelVer,statuscodes,MinRun,MaxRun,desc)

def getControlJobList(name="",BrunelVer="v40r1",statuscodes=['completed'],
                      MinRun=0,MaxRun=99999999,desc=""):
    return getListOfJobs('RefInControl',name,BrunelVer,statuscodes,MinRun,MaxRun,desc)

def nScaleFromShift(shift,rad='Rich1Gas'):

    # As of RICH S/W meeting 3/9/2010
    #slope = 38.2388535346
    #if rad == 'Rich2Gas': slope = 68.2

    # 2011-NewTkRichAlign-V1
    #slope = 38.0
    #if rad == 'Rich2Gas': slope = 68.5

    # Final tuning for 2011 repro
    slope = 38.1
    if rad == 'Rich2Gas': slope = 65.25
    
    # Compute the scale factor and its error
    result = 1.0 + (shift['Mean'][0]*slope)
    error  = shift['Mean'][1]*slope
    
    # Return the values
    return [result,error]

def getRootFilePath(j):
    listofMerged = [ j.outputdir+f for f in j.merger.files ]
    filename = ""
    if len(listofMerged) > 0 : filename = listofMerged[0]
    return filename

def getListOfRootFiles(cjobs):
    names = [ ]
    for j in cjobs:
        name = getRootFilePath(j)
        if name != "" :
            names += [name]
    return names

def dumpRootFileNamesToText(cjobs,filename='RootFileNames.txt'):
    names = getListOfRootFiles(cjobs)
    file = open(filename,'w')
    print "Dumping", len(names), "ROOT file name(s) to", filename
    for name in names : file.write(name+"\n")
    file.close()
    
def getRootFile(j):
    import os
    from ROOT import TFile
    file = None
    filename = getRootFilePath(j)
    if filename != "" :
        if os.path.exists(filename):
            #print "Opening file", filename
            file = TFile( filename )
        else:
            print "ERROR :", filename, "does not exist"
    else:
        print "ERROR Accessing ROOT file for job", j.id
    return file

def removeSubJobOutputData(js):
    import os, shutil
    for j in js:
        if j.status == "completed":
            rootFile = getRootFile(j)
            if rootFile != None:
                rootFile.Close()
                print "Removing subjob output data for job", j.id, j.name
                for subj in j.subjobs:
                    if os.path.exists(subj.outputdir):
                        shutil.rmtree(subj.outputdir)
            else:
                print "WARNING : Job", j.id, "completed but root file cannot be opened"
        else:
            print "Job", j.id, "is not completed. Nothing removed"

def rootCanvas():
    from ROOT import TCanvas
    if globals()["canvas"] == None :
        import RootStyle
        RootStyle.rootStyle()
        globals()["canvas"] = TCanvas("CKCanvas","CKCanvas",1000,750)
    return globals()["canvas"]

def printCanvas(tag=''):
    import os
    canvas = rootCanvas()
    imageType = imageFileName.split(".")[1]
    if tag == "[" :
        if os.path.exists(imageFileName) : os.remove(imageFileName)
        print "Opening file", imageFileName
    if tag != "[" and tag != "]" : canvas.Update()
    canvas.Print(imageFileName+tag,imageType)
    # ROOT built in PDFs look crappy. Better to make PS and convert with ps2pdf ...
    if tag == ']' and imageType == 'ps' :
        print "Converting", imageFileName, "to PDF"
        os.system('ps2pdf '+imageFileName)
        os.remove(imageFileName)

def fitCKExpectedHistogram(rootfile,run,rad='Rich1Gas'):

    from ROOT import TF1, TH1, TText, gROOT

    # Default return result
    result = { 'OK' : False, "Message" : "No Message" }

    # Load the root file for this job
    if rootfile == None :

        result['Message'] = "Failed to open ROOT file"

    else:

        # Parameters
        minEntries = 100
        
        # Get the histogram
        histName = globals()["histoBase"]+rad+'/thetaExpect'
        hist = rootfile.Get(histName)
        if not hist:
            
            print "ERROR : Could not access histogram", histName
            
        else:

            # Draw
            hist.Draw()
            # Add Run number to page
            addRunToPlot(run)
            # Print
            #printCanvas()
                
            # Basic check on the histograms before fitting
            entries = hist.GetEntries()
            if entries < minEntries :

                result['Message'] = "Too few histogram entries"

            else:

                # Mean of the histo
                mean    = hist.GetMean()
                meanerr = hist.GetMeanError()
                
                result = { 'Message' : "Fit OK",
                           'OK'      : True,
                           'Mean'    : [mean,meanerr]
                           }

    return result

def fitCKForFile(filename,plot='ckResAll',outfile="CKFit.pdf"):

    from ROOT import TFile, TH1F, TF1, TH1, TText, gROOT

    file = TFile(filename)

    if file == None :

        print "Failed to open ROOT file", filename

    else:

        # Start a PDF file
        globals()["imageFileName"] = "results/"+outfile
        printCanvas('[')

        fitResult1 = fitCKThetaHistogram(file,0,'Rich1Gas',plot)
        print fitResult1
        fitResult2 = fitCKThetaHistogram(file,0,'Rich2Gas',plot)
        print fitResult2

        # Close PDF file
        printCanvas(']')

        file.Close()
        
def fitCKThetaHistogram(rootfile,run,rad='Rich1Gas',plot='ckResAll',nPolFull=3):

    from ROOT import TH1F, TF1, TH1, TText, gROOT

    # Default return result
    result = { 'OK' : False, "Message" : "No Message" }

    # Load the root file for this job
    if rootfile == None :

        result['Message'] = "Failed to open ROOT file"

    else:

        # Parameters
        minEntries = 10000

        # Get the histogram
        histName = globals()["histoBase"]+rad+'/'+plot
        hist = rootfile.Get(histName)
        if not hist :
            
            print "ERROR : Could not access histogram", histName

        else:

            # Basic check on the histograms before fitting
            entries = hist.GetEntries()
            if entries < minEntries :

                result['Message'] = "Too few histogram entries"
                
            else:

                preFitColor  = 12
                fullFitColor = 2
                bkgColor     = 4

                # Get x value of highest content bin
                # (rough estimate of peak position)
                xPeak = hist.GetBinCenter(hist.GetMaximumBin())

                # Pre Fitting range
                delta = 0.0025
                if rad == 'Rich2Gas' : delta = 0.00105
                fitMin = xPeak - delta
                fitMax = xPeak + delta

                # Gaussian function
                preFitFType = "gaus"
                preFitF = TF1(rad+"PreFitF",preFitFType,fitMin,fitMax)
                preFitF.SetLineColor(preFitColor)
                # Starting params
                preFitF.SetParameter(1,0)
                if rad == 'Rich1Gas' :
                    preFitF.SetParameter(2,0.0015)
                else:
                    preFitF.SetParameter(2,0.0007)
 
                # Do the pre fit with just a Gaussian
                hist.Fit(preFitF,"QRS0")

                # Full Fitting range
                if rad == 'Rich1Gas' :
                    fitMax = xPeak+0.0069
                    fitMin = xPeak-0.0088
                else:
                    fitMax = xPeak+0.0036
                    fitMin = xPeak-0.0044

                # Loop over pol fits up to the final
                lastFitF = preFitF
                bestFitF = preFitF
                bestNPol = nPolFull
                fitOK    = True
                if nPolFull>0 :
                    for nPol in xrange(1,nPolFull+1):
                        fFuncType = "gaus(0)+pol"+str(nPol)+"(3)"
                        fFitF = TF1(rad+"FitF"+str(nPol),fFuncType,fitMin,fitMax)
                        fFitF.SetLineColor(fullFitColor)
                        fFitF.SetParName(0,"Gaus Constant")
                        fFitF.SetParName(1,"Gaus Mean")
                        fFitF.SetParName(2,"Gaus Sigma")
                        nParamsToSet = 3
                        if nPol > 1 : nParamsToSet = 3+nPol
                        for p in xrange(0,nParamsToSet) :
                            fFitF.SetParameter(p,lastFitF.GetParameter(p))
                        hist.Fit(fFitF,"QRSE0")
                        lastFitF = fFitF
                        # Fit OK ?
                        maxErrorForOK = 1e-3
                        # gMinuit = gROOT.GetGlobal( "gMinuit", 1 )
                        # print gMinuit.GetStatus()
                        fitOK = fFitF.GetParError(1) < maxErrorForOK
                        if fitOK :
                            bestFitF = fFitF
                            bestNPol = nPol
                        else:
                            if nPol == nPolFull:
                                # Use last good fit
                                print "Pol", nPol, "fit failed...."
                                hist.Fit(fFitF,"RSE0")
                                print " -> Going back to Pol", nPol-1, "fit"
                                fitOK = True
                            if nPol > 1 : break
                  
                # Draw the histogram
                hist.Draw()

                # Draw the full fit
                bestFitF.Draw('SAME')
                
                # Background function
                if bestNPol > 0 :
                    bkgFunc = TF1( rad+"BkgF", "pol"+str(bestNPol), fitMin, fitMax )
                    bkgFunc.SetLineColor(bkgColor)
                    for n in range(0,bestNPol+1):
                        bkgFunc.SetParameter(n,bestFitF.GetParameter(n+3))
                    # Draw the background shape
                    bkgFunc.Draw('SAME')
                       
                # Add Run number to plot
                if bestNPol > 0 :
                    addRunToPlot(run,[ ("Signal+Bkg Fit",fullFitColor),
                                       ("Bkg pol"+str(bestNPol),bkgColor) ] )
                else:
                    addRunToPlot(run)
            
                # Print to file
                if plot == 'ckResAll' :
                    printCanvas()
    
                # Results of the fit
                if fitOK :
                    result = { 'Message' : "Fit OK",
                               'OK'      : True,
                               'Mean'    : [bestFitF.GetParameter(1),bestFitF.GetParError(1)],
                               'Sigma'   : [abs(bestFitF.GetParameter(2)),bestFitF.GetParError(2)]
                               }
                else:
                    result['Message'] = "Histogram Fit Failed"

    # Return the fit result
    return result

def averageCPUTimePerEvent(jobs):

    from ROOT import TH1F

    globals()["imageFileName"] = "CPUTime.png"

    # Root canvas
    rootCanvas()

    # Book a histogram
    minTime = 0
    maxTime = 100000
    cpuHist = TH1F( "NormCPUTime", "Normalised Total CPU Time",
                    100, minTime, maxTime )

    for job in jobs:
        for sjob in job.subjobs:

            # Get the CPU time of this job
            cpuTime = sjob.backend.normCPUTime

            # Fill the plots
            if cpuTime != None :
                fcpuTime = float(cpuTime)
                cpuHist.Fill(fcpuTime)
                if fcpuTime > maxTime :
                    print "Job %s.%s CPU time %s > HistMax %s" % (job.id,sjob.id,fcpuTime,maxTime)
            
    # Print plot to PNG
    cpuHist.Draw('E')
    printCanvas()
    
def addRunToPlot(run,tags=[]):
    from ROOT import TText
    text = TText()
    text.SetNDC()
    text.SetTextSize(0.03)
    x = 0.12
    y = 0.85
    text.DrawText( x, y, "Run "+str(run) )
    for tag in tags:
        if tag[0] != "":
            y -= 0.05
            text.SetTextColor(tag[1])
            text.DrawText( x, y, tag[0] )
    
def getRunLFNData(pickedRuns):
    import pickle, bz2
    file = bz2.BZ2File(pickedRuns,"r")
    RunLFNs = pickle.load(file)
    file.close()
    return RunLFNs

def drange(start,stop,step):
    r = start
    while r < stop:
        yield r
        r += step

def dateTimeString():
    import datetime
    now = datetime.datetime.now()
    return str(now.year)+'-'+str(now.month)+'-'+str(now.day)+'_'+str(now.hour)+'-'+str(now.minute)+'-'+str(now.second)

def createTempOptsFile(name):
    import os
    tempFullPath = os.getcwd()+"/tempOpts"
    if not os.path.exists(tempFullPath) : os.makedirs(tempFullPath)
    return tempFullPath+"/"+name+"_"+dateTimeString()+".py"

def mtime(filename):
    import os
    return os.stat(filename).st_mtime

def getPickledRunList():
    import glob
    return sorted(glob.glob('RunData/*.pck.bz2'),key=mtime)

def deleteJobsWithBadRootFile(cjobs,rad='Rich1Gas'):

    djobs = [ ]

    for j in cjobs:

        # Root file
        rootfile = getRootFile(j)

        histName = globals()["histoBase"]+rad+'/thetaExpect'
        hist = rootfile.Get(histName)
        if not hist :
            print " -> Bad ROOT file. Will delete."
            djobs += [j]

        rootfile.Close()

    if len(djobs) == 0 :
        print "No jobs to delete"
    else:
        print "Jobs to delete :", djobs
        for j in djobs : j.remove()

def checkInputDataReplicas(lfns):
    OK = True
    for lfn in lfns :
        res = lfn.getReplicas()
        if len(res) == 0 :
            print "ERROR : LFN", lfn.name, "has no replicas"
            OK = False
    return OK

def filesPerJob(nFiles):
    if nFiles == 1  : return 1
    if nFiles == 2  : return 2
    if nFiles == 3  : return 3
    if nFiles == 4  : return 2
    if nFiles == 5  : return 3
    if nFiles == 6  : return 3
    if nFiles == 7  : return 3
    if nFiles == 8  : return 4
    if nFiles == 9  : return 3
    if nFiles == 10 : return 5
    if nFiles == 12 : return 4
    if nFiles == 18 : return 6
    if nFiles < 20  : return 5
    if nFiles < 100 : return 6
    return 10

def removeCalibrationDataSet(name,BrunelVer="v40r1"):
    from Ganga.GPI import jobtree
    js = getCalibrationJobList(name,BrunelVer,
                               statuscodes=['completed','running','submitted','failed'])
    for j in js : j.remove()
    path = '/RichCalibration/RefInCalib-'+name+'_BR-'+BrunelVer
    if jobtree.exists(path) : jobtree.rm(path)
    jobtree.cd('/RichCalibration')

def removeVerificationDataSet(name,BrunelVer="v40r1"):
    from Ganga.GPI import jobtree
    js = getVerificationJobList(name,BrunelVer,
                               statuscodes=['completed','running','submitted','failed'])
    for j in js : j.remove()
    path = '/RichCalibration/RefInVerify-'+name+'_BR-'+BrunelVer
    if jobtree.exists(path) : jobtree.rm(path)
    jobtree.cd('/RichCalibration')

#=============================================================================================

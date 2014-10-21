# coding=utf-8
"""
Module with functions to create pdf pages using matplotlib
"""
__author__ = "Pieter David <pieter.david@nikhef.nl>"
__date__   = "November 2012"

__all__  = ( "ElementGroups"
           , "drawTimeTrends"
           )

import os
import re
import logging
import datetime
import pytz
import math

import matplotlib as mpl
mpl.rcParams["backend"] = "PDF"
mpl.rcParams["font.size"] = 8
import matplotlib.pyplot as plt
import matplotlib.dates
import matplotlib.ticker
from matplotlib.figure import SubplotParams
from matplotlib.patches import Rectangle
from matplotlib.collections import PatchCollection
import numpy as np

class Region(object):
    """
    Base for PlotRegion and ROOTRegion
    """
    def __init__(self, name, var, parent):
        self.name = name
        self.var = var
        self.parent = parent

    def getFullName(self):
        return "".join((self.parent.getFullName(), self.name))

    def addAlignment(self, parValues, **drawOptions):
        pass
    
    def addReference(self, parValues, **drawOptions):
        pass

    def draw(self):
        pass

class PlotRegion(Region):
    """
    A class representing a "plot region" on a page (axes)

    Knows how to make one plot.
    @see the subclasses for time trends (TrendPlotRegion) and diffs (DiffPlotRegion)
    """
    def __init__(self, name, var, parent, axes):
        super(PlotRegion, self).__init__(name, var, parent)

        self.axes = axes

    def makeYLabel(self):    
        if self.var.startswith("T"):
            self.axes.set_ylabel("%s (mm)" % self.var)
        elif self.var.startswith("R"):
            self.axes.set_ylabel("%s (mrad)" % self.var)
        else:
            self.axes.set_ylabel(self.var)

def roundUp( num, resolution=1.0 ):
    return math.ceil( num / resolution )*resolution

def roundDown( num, resolution=1.0 ):
    return math.floor( num / resolution )*resolution

class TrendPlotRegion(PlotRegion):
    DefaultFormats = { "MagUp"   : "r-"
                     , "MagDown" : "b-"
                     , "MagOff"  : "g-"
                     , "TS"      : { "facecolor" : "k" , "alpha" : 0.05 , "lw" : 0.0 }
                     , "MD"      : { "facecolor" : "k" , "alpha" : 0.05 , "lw" : 0.0 }
                     , "VDM"     : { "facecolor" : "k" , "alpha" : 0.05 , "lw" : 0.0 }
                     }
    def __init__(self, name, var, parent, axes, timezone="CET", periods=None):
        self.timezone = timezone
        self.periods  = periods
        self.mn = None
        self.mx = None
        super(TrendPlotRegion, self).__init__(name, var, parent, axes)

    def draw(self):
        super(TrendPlotRegion, self).draw()

        res = 1.0
        if self.var.startswith("T"):
            res = 0.1
        elif self.var.startswith("R"):
            res = 0.01

        dmnmx = self.mx-self.mn
        dmin = roundDown( self.mn-.1*dmnmx, res )
        dmax = roundUp( self.mx+.1*dmnmx, res )
        self.axes.set_ylim( dmin, dmax )
        if dmnmx < 5.*res:
            self.axes.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(res))
        elif dmnmx < 100.*res:
            self.axes.yaxis.set_minor_locator(matplotlib.ticker.MultipleLocator(res))

        self.axes.xaxis.set_minor_locator( matplotlib.dates.MonthLocator() )

        tz = pytz.timezone(self.timezone)
        for period in self.periods:
            if not period.status.needAlignment:
                self.axes.axvspan( period.startTime.astimezone(tz), period.endTime.astimezone(tz), **(TrendPlotRegion.DefaultFormats[period.status.name]))
        self.axes.set_title(self.name.replace("_","\n"))

    def addAlignment(self, parValues, addStats=False, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValues = list( 1000.*p for p in parValues )
        # apply *after* rad->mrad so that the cutoff is 0.1 micron/micro rad
        if "Velo" in self.getFullName():
            parValues = list( round(p,4) for p in parValues )

        if not "solid_capstyle" in drawOptions:
            drawOptions["solid_capstyle"] = "butt"

        if not len(parValues) == sum(1 for p in self.periods if p.status.needAlignment):
            logging.error("Too few parameter values, check!!")
            return
        tz = pytz.timezone(self.timezone)
        curPar = None
        curParIt = iter(parValues)
        for period in self.periods:
            if period.status.needAlignment:
                curPar = curParIt.next()
                fmt = drawOptions.pop("fmt", TrendPlotRegion.DefaultFormats[period.status.name])
                logging.debug("Adding alignment for period %s -- %s (localized to %s: %s -- %s)", period.startTime, period.endTime, self.timezone, period.startTime.astimezone(tz), period.endTime.astimezone(tz))
                self.axes.plot( (period.startTime.astimezone(tz), period.endTime.astimezone(tz)), (curPar, curPar), fmt, **drawOptions )

        if addStats:
            avg = sum( p for p in parValues ) / len(parValues)
            rms = math.sqrt( sum( (p-avg)*(p-avg) for p in parValues ) / len(parValues) )
            self.axes.text( 1., 1.02, u"%.02f±%.02f" % (round(avg,2), round(rms,2)), ha="right", va="baseline", transform=self.axes.transAxes )

        # axis limits
        if self.mn is not None and self.mx is not None:
            self.mn, self.mx = min( min(parValues), self.mn ), max( max(parValues), self.mx )
        else:
            self.mn, self.mx = min(parValues), max(parValues)

    def addReference(self, parValue, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue

        drawOptions["alpha"] = drawOptions.get("alpha", 0.5)
        drawOptions["color"] = drawOptions.get("color", "k")
        self.axes.axhline( parValue, **drawOptions )

        # axis limits
        if self.mn is not None and self.mx is not None:
            self.mn, self.mx = min( parValue, self.mn ), max( parValue, self.mx )
        else:
            self.mn, self.mx = parValue, parValue

class DiffPlotRegion(PlotRegion):
    def __init__(self, name, var, parent, axes):
        self.mn = None
        self.mx = None
        self.labels = list()
        self.refLabels = list()
        self._x = -1.
        self._xr = 0.
        super(DiffPlotRegion, self).__init__(name, var, parent, axes)

    def draw(self):
        super(DiffPlotRegion, self).draw()

        self.axes.set_xlim(self._xr-.5, self._x+.5)
        self.axes.set_xticks(range(-len(self.refLabels), len(self.labels)))
        self.axes.set_xticklabels(list(l for l in reversed(self.refLabels)) + self.labels)

        self.axes.set_title(self.name.replace("_","\n"))

    def addAlignment(self, parValue, label=None, draw="P", **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue
        # precision cutoff for Velo
        # apply *after* rad->mrad so that the cutoff is 0.1 micron/micro rad
        if "Velo" in self.getFullName():
            parValue = round(parValue,4)

        if draw == "P":
            self.labels.append(label)
            self._x += 1.
            self.axes.plot( ( self._x, ), ( parValue, ), drawOptions.pop("fmt", "k."), **drawOptions )
        elif draw == "L":
            self.axes.axhline( parValue, **drawOptions )

    def addReference(self, parValue, label=None, draw="L", **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue

        if draw == "P":
            self.refLabels.append(label)
            self._xr -= 1.
            self.axes.plot( ( self._xr, ), ( parValue, ), drawOptions.pop("fmt", "k."), **drawOptions )
        elif draw == "L":
            self.axes.axhline( parValue, **drawOptions )

class Page(object):
    """
    A class representing a "page" (maps to a TCanvas)

    Container of plot region, main task is to delegate things to the right one.
    Additionally handles output to a file
    """
    def __init__(self, name, parent, nRows, nCols):
        """
        Constructor
        """
        self.name = name
        self.parent = parent

        self.nRows = nRows
        self.nCols = nCols
        self.fig = plt.figure(figsize=(11.6929134, 8.26771654), subplotpars=SubplotParams(wspace=0.35, left=0.1, bottom=0.2, right=0.98))
        self._l = 0

        self.regions = dict()

    def getFullName(self):
        return "".join((self.parent.getFullName(), self.name))

    def addRegion(self, name, var, **kwargs):
        self._l += 1
        self.regions[name] = kwargs.pop("RegionType", PlotRegion)(name, var, self, self.fig.add_subplot(self.nRows, self.nCols, self._l), **kwargs)
        topInCol = self.fig.add_subplot(self.nRows, self.nCols, ((self._l-1) % self.nCols)+1 )
        self.regions[name].axes.sharex = topInCol
        # from pylab import setp
        # setp( self.regions[name].axes.get_xticklabels(), visible=True )
        if (self._l % self.nCols) == 1:
            self.regions[name].makeYLabel()

    def __getitem__(self, name):
        return self.regions.__getitem__(name)

    def save(self, outputFileName):
        self.fig.suptitle(self.name)
        for reg in self.regions.itervalues():
            reg.draw()
        self.fig.autofmt_xdate()
        self.fig.tight_layout(pad=3,h_pad=1.08,w_pad=1.08)
        self.fig.savefig(outputFileName)

class Folder(object):
    """
    A class representing a folder - a set of pages

    Container of pages, may be used to create grouped output
    """
    def __init__(self, name):
        """
        Constructor
        """
        self.name = name
        self.pages = dict()

    def getFullName(self):
        return self.name

    def addPage(self, name, *args):
        pass

    def __getitem__(self, name):
        return self.pages.__getitem__(name)

    def save(self, output="."):
        pass

class FSFolder(Folder):
    """
    Folder that saves its pages in directories on the filesystem
    """
    def __init__(self, name):
        super(FSFolder, self).__init__(name)

    def addPage(self, name, nRows, nCols):
        self.pages[name] = Page(name, self, nRows, nCols)

    def save(self, output=".", extension="pdf"):
        outputDirFull = os.path.join(*([ output ] + self.name.split(".")[:2]))
        if not os.path.isdir(outputDirFull):
            os.makedirs(outputDirFull)
        for n, p in self.pages.iteritems():
            logging.info("Saving page %s to directory %s (%s)" % ( self.getFullName(), outputDirFull, ".".join((n,extension)) ))
            p.save(os.path.join(outputDirFull, ".".join((n, extension))))

### Equivalent classes for saving TGraphs instead of making plots with matplotlib

class ROOTRegion(Region):
    """
    Region, base for ROOTTrendRegion and ROOTDiffRegion
    """
    def __init__(self, name, var, parent):
        super(ROOTRegion, self).__init__(name, var, parent)
        self.objs = []

    def save(self, output=None):
        import ROOT
        assert isinstance( output, ROOT.TDirectoryFile ) and output.IsWritable()
        reqPath = "/".join((self.name, self.var))
        output.cd()
        if not output.GetDirectory(reqPath):
            assert output.mkdir(reqPath)
        myDir = output.GetDirectory(reqPath)
        myPath = myDir.GetPath()
        myDir.cd()
        for g in self.objs:
            g.Write()

class ROOTTrendRegion(ROOTRegion):
    def __init__(self, name, var, parent, timezone="CET", periods=None):
        self.timezone = timezone
        self.periods  = periods
        self.alignments = []
        self.references = []
        self.mn = None
        self.mx = None
        super(ROOTTrendRegion, self).__init__(name, var, parent)

    def addAlignment(self, parValues, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValues = list( 1000.*p for p in parValues )
        # apply *after* rad->mrad so that the cutoff is 0.1 micron/micro rad
        if "Velo" in self.getFullName():
            parValues = list( round(p,4) for p in parValues )

        if not len(parValues) == sum(1 for p in self.periods if p.status.needAlignment):
            logging.error("Too few parameter values, check!!")
            return

        self.alignments.append( parValues )

    def addReference(self, parValue, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue

        self.references.append( parValue )

    def save(self, output=None):
        tz = pytz.timezone(self.timezone)
        epoch = datetime.datetime(1995, 1, 1, 0, 0, 0, 0, tz)
        def toROOTTime(someTime):
            # seconds since 1995
            return (someTime-epoch).total_seconds()

        from ParseStatusTable import StatusTimePeriod

        import ROOT
        colors = { StatusTimePeriod.MagUp : ROOT.kRed, StatusTimePeriod.MagDown : ROOT.kBlue, StatusTimePeriod.MagOff : ROOT.kBlack }

        mnTime, mxTime = None, None
        for alignment in self.alignments:
            entriesPerStatus = { StatusTimePeriod.MagUp : [], StatusTimePeriod.MagDown : [], StatusTimePeriod.MagOff : [] }

            valsIt = iter(alignment)
            for period in self.periods:
                if period.status.needAlignment:
                    val = next(valsIt)
                    entriesPerStatus[period.status].append([toROOTTime(period.startTime.astimezone(tz)), toROOTTime(period.endTime.astimezone(tz)), val])

            mg = ROOT.TMultiGraph()
            for status, entries in entriesPerStatus.iteritems():
                if len(entries) == 0:
                    continue
                beginTimes, endTimes, values = tuple(np.array(x) for x in zip(*entries))
                mnTime = min(mnTime, min(beginTimes)) if mnTime is not None else min(beginTimes)
                mxTime = max(mxTime, max(endTimes)) if mxTime is not None else max(endTimes)
                g = ROOT.TGraphErrors(values.shape[0], 0.5*(beginTimes+endTimes), values, 0.5*(endTimes-beginTimes), np.zeros(values.shape))
                g.SetName("_".join((self.name, status.name)))
                g.GetXaxis().SetTimeDisplay(True)
                g.SetMarkerColor(colors[status])
                g.SetLineColor(colors[status])
                mg.Add(g, "EZ")
            mg.SetName(self.name)
            mg.SetTitle(" ".join((self.getFullName(), self.var)))
            self.objs.append(mg)

        for i, ref in enumerate(self.references):
            g = ROOT.TGraphErrors(1, np.array([0.5*(mnTime+mxTime)]), np.array([ref]), np.array([0.5*(mxTime-mnTime)]), np.array([0.]))
            g.SetName("_".join((self.name, "ref", str(i))))
            g.GetXaxis().SetTimeDisplay(True)
            self.objs.append(g)

        super(ROOTTrendRegion, self).save(output=output)

class ROOTDiffRegion(ROOTRegion):
    def __init__(self, name, var, parent):
        self._x = -1.
        self._xr = 0.
        self.x = []
        self.y = []
        self.labels = []
        self.xRef = []
        self.yRef = []
        self.refLabels = []
        super(ROOTDiffRegion, self).__init__(name, var, parent)

    def addAlignment(self, parValue, label=None, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue
        # apply *after* rad->mrad so that the cutoff is 0.1 micron/micro rad
        if "Velo" in self.getFullName():
            parValue = round(parValue,4)

        self._x += 1.
        self.labels.append(label if label is not None else str(self._x))
        self.x.append(self._x)
        self.y.append(parValue)

    def addReference(self, parValue, label=None, **drawOptions):
        # rad -> mrad
        if self.var.startswith("R"):
            parValue = 1000.*parValue

        self._xr -= 1.
        self.refLabels.append(label if label is not None else str(self._xr))
        self.xRef.append(self._xr)
        self.yRef.append(parValue)

    def save(self, output=None):
        import ROOT
        h = ROOT.TH1F()
        h.SetBins(len(self.x)+len(self.xRef), -len(self.xRef), len(self.x))
        h.SetName(self.name)
        h.SetTitle(self.name.replace("_", " "))
        axis = h.GetXaxis()
        from itertools import izip
        for ix,iy,iLbl in izip(self.x, self.y, self.labels):
            b = h.FindBin(ix)
            h.SetBinContent(b, iy)
            axis.SetBinLabel(b, iLbl)
        for ix,iy,iLbl in izip(self.xRef, self.yRef, self.refLabels):
            b = h.FindBin(ix)
            h.SetBinContent(b, iy)
            axis.SetBinLabel(b, iLbl)
        self.objs.append(h)
        super(ROOTDiffRegion, self).save(output=output)

class ROOTFolder(Folder):
    """
    Save pages in directories of a ROOT file
    """
    def __init__(self, name):
        super(ROOTFolder, self).__init__(name)

    def addPage(self, name, *args):
        self.pages[name] = ROOTFolder(name)

    def save(self, output=None):
        import ROOT
        assert isinstance( output, ROOT.TDirectoryFile ) and output.IsWritable()
        reqPath = self.name.split("__")[0].replace("_", "/")
        output.cd()
        if not output.GetDirectory(reqPath):
            assert output.mkdir(reqPath)
        myDir = output.GetDirectory(reqPath)
        myPath = myDir.GetPath()
        myDir.cd()
        for n, p in self.pages.iteritems():
            logging.info("Saving page %s to directory %s" % ( p.name, myPath ))
            p.save(myDir)

    ## Also a Page, actually
    def addRegion(self, name, var, **kwargs):
        self.pages[name] = kwargs.pop("RegionType", ROOTRegion)(name, var, self, **kwargs)

    @property
    def regions(self):
        return self.pages

class ElementGroup(object):
    """
    Helper class to figure out from the element name if we want to draw it
    and to which page it should go (match method returns the right page or None).
    """
    def __init__(self, expr, nRows, nCols, suffix=""):
        self.regex  = re.compile(expr)
        self.nRows  = nRows
        self.nCols  = nCols
        self.suffix = suffix
    def match(self, elementName):
        m = self.regex.match(elementName)
        if m is None:
            return None
        else:
            return ( m.group("page").replace("/", "_")+self.suffix, m.group("elm").replace("/", "_") )

ElementGroups = { "TT.Layers"       : ElementGroup("(?P<page>TT)/TT[ab]/(?P<elm>TT(?:aX|aU|bV|bX)Layer)$", 2, 2)
                , "TT.Modules"      : ElementGroup("TT/TT[ab]/(?P<page>TT(?:aX|aU|bV|bX)Layer/R[1-3])Module(?P<elm>[1-6][BT])$", 4, 3)
                , "TT.Sensors"      : ElementGroup("TT/TT[ab]/(?P<page>TT(?:aX|aU|bV|bX)Layer/R[1-3]Module[1-6][BT]/Ladder[0-9])/(?P<elm>Sensor[0-9]*)$", 2, 2)

                , "IT.Boxes"        : ElementGroup("IT/(?P<page>Station[1-3])/(?P<elm>(?:ASide|CSide|Top|Bottom)Box)$", 2, 2)
                , "IT.Layers"       : ElementGroup("IT/(?P<page>Station[1-3]/(?:ASide|CSide|Top|Bottom)Box)/(?P<elm>Layer(?:X1|U|V|X2))$", 2, 2)
                , "IT.Ladders"      : ElementGroup("IT/(?P<page>Station[1-3]/(?:ASide|CSide|Top|Bottom)Box/Layer(?:X1|U|V|X2))/(?P<elm>Ladder[1-7])$", 3, 3)
                # , "IT.Sensors.BottomTop" : ElementGroup("IT/(?P<page>Station[1-3]/(?:Top|Bottom)Box/Layer(?:X1|U|V|X2))/(?P<elm>Ladder[1-7]/Sector/Sensor[1-2])$", 2, 4)### redundant
                , "IT.Sensors" : ElementGroup("IT/(?P<page>Station[1-3]/(?:A|C)SideBox/Layer(?:X1|U|V|X2)/Ladder[1-7])/Sector/(?P<elm>Sensor[1-2])$", 1, 2)# sides only

                  # 'CFrameLayers' are just layers joined in X1+U and V+X2 pairs
                , "OT.CFrameLayers" : ElementGroup("OT/(?P<page>T[1-3])/(?P<elm>(?:X1|U|V|X2))$", 2, 2)
                  # 'CFrames' are just quarters joined in layer X1+U and V+X2 pairs
                , "OT.CFrames"      : ElementGroup("OT/(?P<page>T[1-3]/(?:X1|U|V|X2))/(?P<elm>Q[0-3])$", 2, 2)
                , "OT.Modules"      : ElementGroup("OT/(?P<page>T[1-3]/(?:X1|U|V|X2)/Q[0-3])/(?P<elm>M[1-9])$", 3, 3)

                , "Velo.Velo"     : ElementGroup("(?P<page>(?P<elm>Velo))$", 1, 2)

                , "Velo.Halves"     : ElementGroup("(?P<page>Velo)/(?P<elm>Velo(?:Left|Right))$", 1, 2)

                , "Velo.Modules"    : [ "Velo.Modules.a", "Velo.Modules.b" ]
                , "Velo.Modules.a"  : ElementGroup("Velo/(?P<page>Velo(?:Left|Right))/(?P<elm>Module(?:[0-1][0-9]|2[0-1]))$", 3, 4, "_a")
                , "Velo.Modules.b"  : ElementGroup("Velo/(?P<page>Velo(?:Left|Right))/(?P<elm>Module(?:2[2-9]|[3-4][0-9]))$", 3, 4, "_b")
                  
                , "Velo.Sensors"     : [ "Velo.Sensors.LRa", "Velo.Sensors.LRb", "Velo.Sensors.LPa", "Velo.Sensors.LPb",
                                         "Velo.Sensors.RRa", "Velo.Sensors.RRb", "Velo.Sensors.RPa", "Velo.Sensors.RPb" ]
                  ### Left
                  ### R Sensors
                , "Velo.Sensors.LRa" : ElementGroup("Velo/(?P<page>VeloLeft)/(?P<elm>Module(?:00|04|08|12|16|20|24|28|32|36|40)/RPhiPair.{2})/Detector-00$", 3, 4, "_R_a")
                , "Velo.Sensors.LRb" : ElementGroup("Velo/(?P<page>VeloLeft)/(?P<elm>Module(?:02|06|10|14|18|22|26|30|34|38)/RPhiPair.{2})/Detector-01$", 3, 4, "_R_b")
                  ### Phi Sensors
                , "Velo.Sensors.LPa" : ElementGroup("Velo/(?P<page>VeloLeft)/(?P<elm>Module(?:00|04|08|12|16|20|24|28|32|36|40)/RPhiPair.{2})/Detector-01$", 3, 4, "_Phi_a")
                , "Velo.Sensors.LPb" : ElementGroup("Velo/(?P<page>VeloLeft)/(?P<elm>Module(?:02|06|10|14|18|22|26|30|34|38)/RPhiPair.{2})/Detector-00$", 3, 4, "_Phi_b")
                  ### Right
                  ### R Sensors
                , "Velo.Sensors.RRa" : ElementGroup("Velo/(?P<page>VeloRight)/(?P<elm>Module(?:01|05|09|13|17|21|25|29|33|37|41)/RPhiPair.{2})/Detector-00$", 3, 4, "_R_a")
                , "Velo.Sensors.RRb" : ElementGroup("Velo/(?P<page>VeloRight)/(?P<elm>Module(?:03|07|11|15|19|23|27|31|35|39)/RPhiPair.{1,2})/Detector-01$", 3, 4, "_R_b")
                  ### Phi Sensors
                , "Velo.Sensors.RPa" : ElementGroup("Velo/(?P<page>VeloRight)/(?P<elm>Module(?:01|05|09|13|17|21|25|29|33|37|41)/RPhiPair.{2})/Detector-01$", 3, 4, "_Phi_a")
                , "Velo.Sensors.RPb" : ElementGroup("Velo/(?P<page>VeloRight)/(?P<elm>Module(?:03|07|11|15|19|23|27|31|35|39)/RPhiPair.{1,2})/Detector-00$", 3, 4, "_Phi_b")
                }
# TODO implement an "ElementGroupFolder" that has ElementGroup, the regular expressions all methods below in its members

defaultFolders = {}

def getOrCreateFolder(folderName, folders=defaultFolders, folderType=FSFolder):
    folder = folders.get(folderName, None)
    if not folder:
        folder = folderType(folderName)
        folders[folderName] = folder
    return folder

def getRegion( folder, elmGroup, elmPageName, dof, RegionType=TrendPlotRegion, **kwargs ):
    pageName = "__".join((elmPageName[0], dof))
    regName = elmPageName[1]
    # get the page
    if pageName not in folder.pages:
        folder.addPage( pageName, elmGroup.nRows, elmGroup.nCols )
    page = folder[pageName]
    # get the region
    if regName not in page.regions:
        page.addRegion( regName, dof, RegionType=RegionType, **kwargs )
    return page[regName]

def drawTimeTrends( periods, elmGroupName, alignmentsWithIOVs, dofs, timezone="CET", addStats=False, folders=defaultFolders, folderType=FSFolder, regionType=TrendPlotRegion ):
    folder = getOrCreateFolder(elmGroupName, folders=folders, folderType=folderType)
    elmGroup = ElementGroups[elmGroupName]
    elmDetName = elmGroupName.split(".")[0]

    logging.info("Adding time trends for %s of %s to the plots" % ( ", ".join(dofs), elmGroupName ))

    for geomName, elmPageName, matrixForAll in alignmentsWithIOVs.loopWithTimesAndValues(elmDetName, elmGroup, periods):
        for dof in dofs:
            region = getRegion(folder, elmGroup, elmPageName, dof, RegionType=regionType, periods=periods, timezone=timezone )
            logging.debug("Adding constants for element %s: %s to region %s (%s)" % ( geomName, [ getattr(align, dof) for align in matrixForAll ], region, region.name ))
            region.addAlignment( [ getattr(align, dof) for align in matrixForAll ], addStats=addStats )

def drawTimeTrendReference( periods, elmGroupName, alignmentsWithIOVs, dofs, folders=defaultFolders, folderType=FSFolder, regionType=TrendPlotRegion ):
    folder = getOrCreateFolder(elmGroupName, folders=folders, folderType=folderType)
    elmGroup = ElementGroups[elmGroupName]
    elmDetName = elmGroupName.split(".")[0]

    logging.info("Adding references for %s of %s to the plots" % ( ", ".join(dofs), elmGroupName ))

    for geomName, elmPageName, matrix in alignmentsWithIOVs.loopWithTimesAndValues(elmDetName, elmGroup, list(p for p in periods if p.status.needAlignment)[0:1]):
        for dof in dofs:
            region = getRegion(folder, elmGroup, elmPageName, dof, RegionType=regionType, periods=periods )
            region.addReference(getattr(matrix[0], dof))

def drawDiffAlignment( periods, elmGroupName, alignment, dofs, label=None, draw="P", folders=defaultFolders, folderType=FSFolder, regionType=DiffPlotRegion ):
    folder = getOrCreateFolder(elmGroupName, folders=folders, folderType=folderType)
    elmGroup = ElementGroups[elmGroupName]
    elmDetName = elmGroupName.split(".")[0]

    logging.info("Adding alignment for %s of %s to the plots" % ( ", ".join(dofs), elmGroupName ))

    for geomName, elmPageName, matrix in alignment.loopWithTimesAndValues(elmDetName, elmGroup, periods ):
        for dof in dofs:
            region = getRegion(folder, elmGroup, elmPageName, dof, RegionType=regionType )
            for align, period in zip(matrix, periods):
                region.addAlignment(getattr(align, dof), label if label else period.startTime.date().isoformat())
                # region.addAlignment(getattr(align, dof), label = period.startTime.date().isoformat())
            # region.addAlignment(getattr(matrix[0], dof), label=label)

def drawDiffReference( periods, elmGroupName, alignment, dofs, draw="P", folders=defaultFolders, folderType=Folder, regionType=DiffPlotRegion ):
    folder = getOrCreateFolder(elmGroupName, folders=folders, folderType=folderType)
    elmGroup = ElementGroups[elmGroupName]
    elmDetName = elmGroupName.split(".")[0]

    logging.info("Adding reference for %s of %s to the plots" % ( ", ".join(dofs), elmGroupName ))

    for geomName, elmPageName, matrix in alignment.loopWithTimesAndValues(elmDetName, elmGroup, periods ):
        for dof in dofs:
            region = getRegion(folder, elmGroup, elmPageName, dof, RegionType=regionType )
            region.addReference(getattr(matrix[0], dof))

def generateRegionEntries( DictName, detectorName, layerName, regionName, moduleNamePrefix, moduleNameSuffix # most of these are used to build the key string
                         , finalModule # the last module number in the sequence
                         , initialCoords # lower-right point of the number "1" element
                         , width, height
                         , shift # what to add to the initialCoords to generate subsequent elements
                         , rotateY # the y coordinate to rotate about for stereo layers
                         , zorder=0 # matplotlib kwarg to set the drawing order; doesn't work for reasons unknown
                         ):
    """
    Create a sequence of geometry dictionary entries with keys that are meant to match the element node names
    A 'sequence' is a set of adjacent detector elements with numerically indexed names
    """
    
    for i in range(1,finalModule + 1):
        _key = detectorName + "/" + layerName + regionName + moduleNamePrefix + str(i) + moduleNameSuffix
        logging.debug("Writing geometry dictionary entry for %r" % _key)
        DictName[_key] = [ np.copy(initialCoords), width, height , rotateY, zorder ]
        initialCoords += shift

def generateTTGeometryDict( DictName, detector, layer ):
    moduleSuffix = ""
    width = 9.24
    height = 9.46
    shift = np.array([0, height])
    if layer.split("/")[0] == "TTa":
        middleModules = 3
    elif layer.split("/")[0] == "TTb":
        middleModules = 5
    xInitialR1 = - ( 6 + middleModules * 0.5 ) * width
    xInitialR2 = - ( middleModules * 0.5 ) * width
    xInitialR3 = - xInitialR2
    rotateY = 0
    TTLayerSeed = { "/R1"   : ( 6, ( (np.array([xInitialR1, -3*height]),  shift, "B", 3), (np.array([xInitialR1, -7*height]),  shift, "B", 4)
                                   , (np.array([xInitialR1,  2*height]), -shift, "T", 3), (np.array([xInitialR1,  6*height]), -shift, "T", 4)
                                   )
                              )
                  , "/R2"   : ( middleModules, ( (np.array([xInitialR2, -1*height]),  shift, "B", 1)
                                               , (np.array([xInitialR2, -3*height]),  shift, "B", 2), (np.array([xInitialR2, -7*height]),  shift, "B", 4)
                                               , (np.array([xInitialR2,         0]), -shift, "T", 1)
                                               , (np.array([xInitialR2,  2*height]), -shift, "T", 2), (np.array([xInitialR2,  6*height]), -shift, "T", 4)
                                               )
                              )
                  , "/R3"   : ( 6, ( (np.array([xInitialR3, -3*height]),  shift, "B", 3), (np.array([xInitialR3, -7*height]),  shift, "B", 4)
                                   , (np.array([xInitialR3,  2*height]), -shift, "T", 3), (np.array([xInitialR3,  6*height]), -shift, "T", 4)
                                   )
                              )
                  } # region: ( number of modules, ( tuple of ladder entries with format:
                                                           # ( sensor xy point, shift to subsequent sensor, top or bottom, number of sensors) ) )
    # fill the dictionary
    for region, entry in TTLayerSeed.iteritems():
        layerName = layer + region
        for k in range(entry[0]):
            module = "Module" + str(k + 1)
            moduleShift = np.array([k * width, 0])
            for coord, step, half, size in entry[1]:
                name = half + "/Ladder" + str(size) + "/Sensor" + str(size)
                initialCoords = coord + moduleShift
                generateRegionEntries(DictName, detector, layerName, module, name, moduleSuffix, size, initialCoords, width, height, step, rotateY)

    # move central module halves to make room for the beampipe
    centralModule = str( (middleModules + 1) / 2)
    layerName = layer + "/R2"
    module = "Module" + centralModule
    for coord, step, half, size in TTLayerSeed["/R2"][1]:
        name = half + "/Ladder" + str(size) + "/Sensor" + str(size)
        initialCoords = coord + np.array([- width * 0.5 - xInitialR2, - 3.7 * cmp(step[1],0)])
        generateRegionEntries(DictName, detector, layerName, module, name, moduleSuffix, size, initialCoords, width, height, step, rotateY)

    # add correct R2 ladders in the edge modules of TTb;
    # ladders 1 and 2 created above do not physically exist and their module entries will never be asked for by the layer fill list
    if layer.split("/")[0] == "TTb":
        layerName = layer + "/R2"
        _entry = lambda index: TTLayerSeed["/R2"][1][index]
        hackedTogether = ( (_entry(1), 0), (_entry(4), 0), (_entry(1), 4), (_entry(4), 4) )
        for (coord, step, half, unused), k in hackedTogether:
            module = "Module" + str(k + 1)
            moduleShift = np.array([k * width, 0])
            name = half + "/Ladder3/Sensor3"
            initialCoords = coord + moduleShift
            generateRegionEntries(DictName, detector, layerName, module, name, moduleSuffix, 3, initialCoords, width, height, step, rotateY)

def generateITGeometryDict( DictName, detector, layer ):
    finalSensor = 7
    width = 7.557
    height = 10.9
#    overlap = 1.1
    # matplotlib zorder isn't working
    overlap = 0
    shift = np.array([width, 0])
    ITLayerSeed = { "/ASideBox/"  : ( (np.array([ 9.9, -height]), shift, 0, 10), (np.array([9.9, 0]), shift, 0, 10) )
                  , "/BottomBox/" : ( (np.array([-3.5*width, - 2*height + overlap]), shift, -1.5*height + overlap, 1), )
                  , "/CSideBox/"  : ( (np.array([-9.9 - 7*width, -height]), shift, 0, 10), (np.array([-9.9 - 7*width, 0]), shift, 0, 10) )
                  , "/TopBox/"    : ( (np.array([-3.5*width, height - overlap]), shift, 1.5*height - overlap, 1), )
                  } # region      : ( ( sensor1 xy point, shift, roatation y coord, zorder(smaller drawn first) ), ( sensor2 xy point, ... ) )
    
    # fill the dictionary
    splitLayer = layer.split("/")
    for region, coords in ITLayerSeed.iteritems():
        # put box names back in
        layer = splitLayer[0] + region + splitLayer[1]
        for k, ( coord, step, rotateY, drawOrder ) in enumerate(coords):
            sensorName = "/Sector/Sensor" + str(k + 1)
            generateRegionEntries(DictName, detector, layer, "", "/Ladder", sensorName, finalSensor, coord, width, height, step, rotateY, zorder=drawOrder)

def generateOTGeometryDict( DictName, detector, layer ):
    modulePrefix = "M"
    moduleSuffix = ""
    finalModule = 9
    width = 34
    height = 240
    shift = np.array([width, 0])
    rotateY = 0
    OTLayerSeed = { "/Q0/"  : (np.array([-297.5, -240]),  shift)
                  , "/Q1/"  : (np.array([ 263.5, -240]), -shift)
                  , "/Q2/"  : (np.array([-297.5,    0]),  shift)
                  , "/Q3/"  : (np.array([ 263.5,    0]), -shift)
                  } # region: (lower right point of module 1, location of subsequent modules)
    
    # fill the dictionary
    for region, ( coord, step ) in OTLayerSeed.iteritems():
        generateRegionEntries(DictName, detector, layer, region, modulePrefix, moduleSuffix, finalModule, coord, width, height, step, rotateY)

    # ammend to make the hole for the IT and the beampipe
    _edit = lambda x: DictName["OT/" + layer + x]
#    _edit("/Q1/M9")[0] += np.array([width * 0.5, 0])
#    _edit("/Q1/M9")[1] -= width * 0.5
    _edit("/Q0/M9")[1] -= width * 0.5 # make Q0/M9 the thin one instead of Q1/M9. this is how the detector was actually installed
    _edit("/Q2/M9")[1] -= width * 0.5
    for i in range(4):
        _edit("/Q" + str(i) + "/M8")[2] -= 11
        _edit("/Q" + str(i) + "/M9")[2] -= 20
    for i in (2,3):
        _edit("/Q" + str(i) + "/M8")[0] += np.array([0, 11])
        _edit("/Q" + str(i) + "/M9")[0] += np.array([0, 20])    

def generateGeometryDict( detector, layer ):
    _Dict = {}
    if detector == "TT":
        generateTTGeometryDict(_Dict, detector, layer)
    elif detector == "IT":
        generateITGeometryDict(_Dict, detector, layer)
    elif detector == "OT":
        generateOTGeometryDict(_Dict, detector, layer)
    return _Dict

# the method that does all the drawing for plotAlignHeat
def drawHeatPlot( comparisonDescription, detector, layer, dof, outputDir, layerFillList, GeometryDict, diffColorRange, drawNames=False ):
    fig = plt.figure(figsize=(11.6929134, 8.26771654), subplotpars=SubplotParams(wspace=0.35, left=0.1, bottom=0.1, right=0.98))
    ax = fig.add_subplot(111, axisbg='#E6E6E6')
    ax.set_aspect("equal", adjustable="box")
    ax.set_title(comparisonDescription + "\n Detector: %s,   Layer: %s,   Degree of Freedom: %s" % (detector, layer, dof) )
    ax.set_xlabel("A side $\qquad \qquad \qquad \qquad \qquad$ x (cm) $\qquad \qquad \qquad \qquad \qquad$ C side")
#    ax.set_xlabel("x (cm)")
    ax.set_ylabel("y (cm)")
    ax.grid(True, linestyle='-', linewidth=1.5, alpha=0.1)
    # put grid behind polygons
    ax.set_axisbelow(True)
    # reverse x axis to match LHCb coodrinates from VELO perspective
    ax.set_xlim(ax.get_xlim()[::-1])
    
    patches = []
    # values will be overwritten, we just need a numpy array at least as big as the fill list
    colorArray = np.array([x for x in range(len(layerFillList))], dtype=np.float64)

    stereoRotation = 0
    if layer.find("U") != -1: stereoRotation = -5
    if layer.find("V") != -1: stereoRotation =  5

    logging.debug("Building list of alignment elements and color array of corresponding alignment parameters")
    for i, (name, unused, matrix) in enumerate(layerFillList):
        _shape = lambda j: GeometryDict[name][j] # (xy, width, height, rotateY, zorder)
        # nb: with x axis reversed, xy of rectangle is lower right point
        poly = Rectangle(_shape(0), _shape(1), _shape(2), zorder=_shape(4))
        if stereoRotation != 0:
            rotate = mpl.transforms.Affine2D().rotate_deg_around(poly.get_x() + _shape(1)*0.5, _shape(3), stereoRotation)
            poly.set_transform(rotate)
        patches.append(poly)
        colorArray[i] = getattr(matrix, dof)

        # element labels
        if drawNames:
            splitName = name.split("/")
            if detector == "TT":
                elementName = "\n".join(splitName[-3:])
                labelRotation = 0
                textSize = 4
            elif detector == "IT":
                elementName = "\n".join(splitName[-3::2])
                labelRotation = 90
                textSize = 8
            elif detector == "OT":
                elementName = "/".join(splitName[-2:])
                labelRotation = 90
                textSize = 10
            smallAngleShift = 0
            if stereoRotation != 0 and detector != "IT":
                tan = 0.08748866
                smallAngleShift = - (poly.get_y() + _shape(2)*0.5)*tan*cmp(stereoRotation,0)
            elementLabel = plt.text(poly.get_x() + _shape(1)*0.5 + smallAngleShift, poly.get_y() + _shape(2)*0.5
                                   , elementName, verticalalignment='center', horizontalalignment='center', rotation=labelRotation-stereoRotation, size=textSize)

    polyCollection = PatchCollection(patches, cmap=mpl.cm.RdBu)
    polyCollection.set_array(colorArray)
    polyCollection.set_clim([-diffColorRange, diffColorRange])
    ax.add_collection(polyCollection)

    cbar = plt.colorbar(polyCollection)
    if dof.startswith("T"):
        cbar.set_label("%s (mm)" % dof)
    elif dof.startswith("R"):
        cbar.set_label("%s (mrad)" % dof)

    plt.axis('equal')

# this is busted for stereo layers, just putting the labels in the x axis title
#     if detector == "IT":
#         ax.text(ax.get_xlim()[0], 0, '$\quad$A side', horizontalalignment='left', verticalalignment='center')
#         ax.text(ax.get_xlim()[1], 0, '$\!\!\!$ C side', horizontalalignment='right', verticalalignment='center')
#     else:
#         ax.text(ax.get_xlim()[0], 0, 'A side     $\qquad$', horizontalalignment='right', verticalalignment='center')
#         ax.text(ax.get_xlim()[1], 0, '$\quad$C side', horizontalalignment='left', verticalalignment='center')

    detectorOutputDir = os.path.join(*([outputDir] + [detector]))
    if not os.path.isdir(detectorOutputDir):
        os.makedirs(detectorOutputDir)
    layerName = layer.replace("/","_")
    fileName = "_".join((detector,layerName,dof)) + ".pdf"
    outputPath = os.path.join(*([detectorOutputDir] + [fileName]))
    print " Writing %s" % outputPath
    fig.savefig(outputPath)
#    plt.show()

#!/usr/bin/env python
"""
A script to add a project to the Software Configuration DB

"""
import logging
import os
import re

from LbConfiguration.Repository import getRepositories
from LbRelease import rcs
from LbRelease.SvnTools.Project import translateProject, getProjectCmt
from LbUtils.Processes import callCommand
from LbRelease.SoftConfDB.SoftConfDB import SoftConfDB


# SVN Repositories definition
url = str(getRepositories(protocol='anonymous')["lbsvn"])
lbsvn = rcs.connect(url)

gaudiurl = str(getRepositories(protocol='anonymous')["gaudi"])
gaudisvn = rcs.connect(gaudiurl)

diracurl = str(getRepositories(protocol='anonymous')["dirac"])
diracsvn = rcs.connect(diracurl)

def importerTranslateProject(p, v):
    ''' Function needed to prevent LCGCMT to be passed to translateProject
    as that does not work '''
    if p.lower() == "lcgcmt":
        return (p.upper(), v)
    else:
        return translateProject(p, v)

class AppImporter:
    """ Main scripts class for looking up dependencies.
    It inherits from """

    def __init__(self):
        # Creating the SoftConfDB Object
        self.mConfDB = SoftConfDB()
        self.log = logging.getLogger()
        self.installArea = None

    def recurseDependencies(self, project, version, alreadyDone = []):
        """ Recursively traverse all dependencies.
        Dependencies are looked up from SVN using the getDependencyMethod.
        A complete list of tuples (project, version) is returned"""
        self.log.warning("Traversing Deps for %s %s" % (project, version))
        self.log.debug(alreadyDone)
        deps = self.processDependencies(project, version)
        alreadyDone.append((project, version))
        ndeps = deps
        for d in deps:
            if not d in alreadyDone:
                tmpDeps = self.recurseDependencies(d[0], d[1], alreadyDone)
                for td in tmpDeps:
                    if not td in alreadyDone:
                        alreadyDone.append(td)
                    if not td in ndeps:
                        ndeps.append(td)
        return ndeps

    def processDependencies(self, p, v):
        """ Get the dependencies for a single project """
        # Cleanup the project name and version and get the SVN URL
        self.log.warning("getDeps %s %s" % (p, v))
        (proj,ver)=importerTranslateProject(p,v)
        tagpath = ""

        # Getting the project.cmt file with dependencies
        if proj.upper() == "GANGA":
            projcmt = self.getGangaProjectCMT(ver)
        else:
            if proj.upper() == "GAUDI":
                tagpath = gaudisvn.url(proj,ver, isProject=True)
            elif proj.upper() == "LHCBDIRAC" or proj.upper() == "DIRAC":
                tagpath = diracsvn.url(proj,ver, isProject=True)
            else:
                tagpath=lbsvn.url(proj,ver, isProject=True)
            self.log.debug("SVN PATH:" + tagpath)
            # Get the project.cmt file and parse it to retrieve the dependencies
            #if not tagpath.endswith("cmt"):
            #    tagpath += "/cmt"
            projcmt=getProjectCmt(tagpath).strip()
            self.log.warn(tagpath)
        deps = []
        for l in projcmt.splitlines():
            m = re.match("\s*use\s+(\w+)\s+([\w\*]+)", l)
            if m != None:
                dp = m.group(1)
                dv = m.group(2)
                deps.append((dp, dv))

        if len(deps) == 0:
            return deps

        # Creating the parent project in the DB
        corver = ver
        if proj in ver:
            corver = ver.replace(proj + "_", "")
        proj = proj.upper()
        
        self.log.warning("Creating project %s %s" % (proj, ver))
        node_parent = self.mConfDB.getOrCreatePV(proj, corver)
        rev = getPathLastRev(tagpath)
        node_parent["Rev"] = rev

        # Now creating the dependencies
        for (dp, dv) in deps:
            if dp in dv:
                dv = dv.replace(dp + "_", "")
            dp = dp.upper()
            self.log.warning("Find project %s %s" % (dp, dv))
            node_child = self.mConfDB.getOrCreatePV(dp, dv)
            self.log.warning("Adding dependency (%s, %s)-[:REQUIRES]->(%s, %s)" % (proj, ver, dp, dv))
            self.mConfDB.addRequires(node_parent, node_child)
        return deps

    def getGangaProjectCMT(self, gangaVersion):
        """ Reads the Project.cmt file for Ganga """
        if self.installArea == None:
            raise Exception("Can only get Ganga dependencies from an install area! - Set it on the AppImporter first...")

        pname = "GANGA"
        f = open(os.path.join(self.installArea, pname, pname + "_" + gangaVersion, "cmt", "project.cmt"), "r")
        txt = f.read()
        f.close()
        return txt


def getProjectLastRev(project, version):
    ''' Get the latest revision of a project in SVN
    Allows to check for retagging '''
    (proj,ver)=importerTranslateProject(project, version)
    tagpath = ""

    # Getting the project.cmt file with dependencies
    if proj.upper() == "GANGA":
        projcmt = self.getGangaProjectCMT(ver)
    else:
        if proj.upper() == "GAUDI":
            tagpath = gaudisvn.url(proj,ver, isProject=True)
        elif proj.upper() == "LHCBDIRAC" or proj.upper() == "DIRAC":
            tagpath = diracsvn.url(proj,ver, isProject=True)
        else:
            tagpath=lbsvn.url(proj,ver, isProject=True)
    return getPathLastRev(tagpath)
    

def getPathLastRev(path):
    ''' Check the SVN revision of the given SVN path '''
    rev = None
    infostr = callCommand('svn','info',path)[0]
    for l in infostr.splitlines():
        m = re.match("Last Changed Rev: (\d+)", l)
        if m != None:
            rev = m.group(1)
            break
    return rev



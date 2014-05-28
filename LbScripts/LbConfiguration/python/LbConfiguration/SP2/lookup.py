###############################################################################
# (c) Copyright 2013 CERN                                                     #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
__author__ = 'Marco Clemencic <marco.clemencic@cern.ch>'

import os
import logging

# FIXME: when we drop Python 2.4, this should become 'from . import path'
from LbConfiguration.SP2 import path, Error

log = logging.getLogger(__name__)

class NotFoundError(Error):
    '''
    Generic error for configuration elements that are not found.
    '''

class MissingManifestError(NotFoundError):
    '''
    The manifest.xml for a project was not found.
    '''

class MissingProjectError(NotFoundError):
    '''
    A project was not found.
    '''
    def __init__(self, *args):
        super(MissingProjectError, self).__init__(*args)
        self.name, self.version, self.platform, self.path = args
    def __str__(self):
        return 'cannot find project {0} {1} for {2} in {3}'.format(*self.args)

class MissingDataPackageError(NotFoundError):
    '''
    A data package was not found.
    '''
    def __init__(self, *args):
        super(MissingDataPackageError, self).__init__(*args)
        self.name, self.version, self.path = args
    def __str__(self):
        return 'cannot data package {0} {1} in {2}'.format(*self.args)


def findProject(name, version, platform):
    '''
    Find a Gaudi-based project in the directories specified in the 'path'
    variable.

    @param name: name of the project (case sensitive for local projects)
    @param version: version of the project
    @param platform: binary platform id

    @return path to the project binary directory
    '''
    log.debug('findProject(%r, %r, %r)', name, version, platform)
    # standard project suffixes
    suffixes = ['{0}_{1}'.format(name, version),
                os.path.join(name.upper(), '{0}_{1}'.format(name.upper(), version))]
    # special case: with the default 'latest' version we allow the plain name
    if version == 'latest':
        suffixes.insert(0, name)

    bindir = os.path.join('InstallArea', platform)
    for d in [os.path.join(b, s, bindir)
              for b in path
              for s in suffixes]:
        log.debug('check %s', d)
        if os.path.exists(d):
            log.debug('OK')
            return d
    else:
        raise MissingProjectError(name, version, platform, path)

def findDataPackage(name, version):
    '''
    Find a data package in the directories specified in the 'path' variable,
    using, optionally, the standard suffixes 'DBASE', 'PARAM', 'EXTRAPACKAGES'.
    If version is a pattern, the latest version matching the pattern is
    returned.

    @param name: name of the package with "hat" (case sensitive)
    @param version: glob pattern to filter the version

    @return: the path to the data package
    '''
    from fnmatch import fnmatch
    from re import findall

    suffixes = ['', 'EXTRAPACKAGES', 'DBASE', 'PARAM']
    versions = []
    for p in [os.path.join(b, s, name)
              for b in path
              for s in suffixes]:
        if os.path.exists(p):
            lst = os.listdir(p)
            if version in lst:
                # stop searching if we find an exact match
                return os.path.join(p, version)
            versions.extend([(v, p) for v in lst if fnmatch(v, version)])
    if not versions:
        raise MissingDataPackageError(name, version, path)
    # sort the versions found
    versions.sort(key=lambda x: tuple(map(int, findall(r'\d+', x[0]))),
                  reverse=True)
    v, p = versions[0]
    return os.path.join(p, v)

def parseManifest(manifest):
    '''
    Extract the list of required projects and data packages from a manifest.xml
    file.

    @param manifest: path to the manifest file
    @return: tuple with ([projects...], [data_packages...]) as (name, version) pairs
    '''
    from xml.dom.minidom import parse
    m = parse(manifest)
    def _iter(parent, child):
        '''
        Iterate over the tags in <parent><child/><child/></parent>.
        '''
        for pl in m.getElementsByTagName(parent):
            for c in pl.getElementsByTagName(child):
                yield c
    # extract the list of used (project, version) from the manifest
    used_projects = [(p.attributes['name'].value, p.attributes['version'].value)
                     for p in _iter('used_projects', 'project')]
    # extract the list of data packages
    data_packages = [(p.attributes['name'].value, p.attributes['version'].value)
                     for p in _iter('used_data_pkgs', 'package')]
    return (used_projects, data_packages)

def getEnvXmlPath(project, version, platform):
    '''
    Return the list of directories to be added to the Env XML search path for
    a given project.
    '''
    pdir = findProject(project, version, platform)
    search_path = [pdir]
    # manifests to parse
    manifests = [os.path.join(pdir, 'manifest.xml')]
    while manifests:
        manifest = manifests.pop(0)
        if not os.path.exists(manifest):
            raise MissingManifestError(manifest)
        projects, packages = parseManifest(manifest)
        # add the data package directories
        search_path.extend([findDataPackage(p, v) for p, v in packages])
        # add the project directories ...
        pdirs = [findProject(p, v, platform) for p, v in projects]
        search_path.extend(pdirs)
        # ... and their manifests to the list of manifests to parse
        manifests.extend([os.path.join(pdir, 'manifest.xml') for pdir in pdirs])
    def _unique(iterable):
        returned = set()
        for i in iterable:
            if i not in returned:
                returned.add(i)
                yield i
    return list(_unique(search_path))


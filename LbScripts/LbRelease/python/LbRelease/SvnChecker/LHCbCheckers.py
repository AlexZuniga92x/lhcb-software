'''
Module containing the standard LHCb checker instances.

The content of the module is used in the LHCb pre-commit script
(https://svnweb.cern.ch/trac/admin/browser/lhcb/usr-hooks/svn-hooks/pre-commit-check.py).

Created on Nov 25, 2010
'''
__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

from StdCheckers import AllPaths, NotContains, PackageTag, ProjectTag
from StdCheckers import TagIntermediateDirs, TagRemoval, AllowedUsers, ValidXml
from StdCheckers import ValidJSON, ValidPythonEncoding, ValidPythonTabs
from StdCheckers import OnPath, MovePackage
from StdCheckers import PropertyChecker
from StdCheckers import tagsFilter
from Core import Failure, Rephrase

__all__ = ("notHasTags", "validTag", "movePackage", "librarian",
           "nightlyConf", "validPython",
           "uniquePackages")

# The transaction does not involve tags:
#   - AllPaths(Failure(), pattern) returns a failure is
notHasTags = NotContains(tagsFilter)

# Standard tag validation policy
validTag = AllPaths(TagRemoval() + TagIntermediateDirs() + ProjectTag() + PackageTag(),
                    tagsFilter)

# Package move
movePackage = MovePackage()

# Check for the librarian account (used to give super user powers to it).
librarian = Rephrase(AllowedUsers(["liblhcb"]),
                     '(can be overridden by the librarian)')

# Run the XML and JSON checkers on the nightly build configuration.
nightlyConf = Rephrase(AllPaths(ValidXml(),
                                r".*LHCbNightlyConf/trunk/configuration\.xml$") *
                       AllPaths(ValidJSON(),
                                r".*LHCbNightlyConf/trunk/.*\.json$"),
                       "Error in Nightly Build configuration file.")

# Check that all Python files do have the right encoding and tabs
validPython = AllPaths(ValidPythonEncoding() * ValidPythonTabs(), r".*\.py$")

# Check that a package name exists only once in the repository (only in the 'packages' property)
def allUnique(packages):
    names = set()
    for l in packages.splitlines():
        l = l.strip()
        if (not l) or l.startswith("#"):
            # Skip empty lines and comments
            continue
        pack, _ = l.split() # extract the package name
        name = pack.rsplit("/",1)[-1] # strip the 'hat'
        if name in names: # Check if the package name was already found
            return False
        else:
            names.add(name)
    return True
uniquePackages = OnPath("/", PropertyChecker("packages", allUnique)) \
                 + Failure(msg = "Duplicate package name in property 'packages'")


def main():
    '''
    Function called by the pre-commit-check in the SVN server.
    '''
    from Core import run

    # Operators meanings:
    #  + -> or
    #  * -> and
    #  unary - -> not

    # checks involving "tags"
    tagsChecks = Rephrase(notHasTags + (movePackage + validTag),
                          "Invalid operation on a tag")

    # normal combination of checkers
    normalCheck = tagsChecks * nightlyConf * validPython

    # Run the checker (on the transaction)
    run(normalCheck + librarian)

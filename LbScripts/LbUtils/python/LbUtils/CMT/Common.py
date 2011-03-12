# package import
from LbUtils.Env import getDefaultEnv
from LbUtils.Processes import callCommand

# global imports
import logging
import os


def setCMTPathEnv(cmtpath=None, cmtprojectpath=None):
    env = getDefaultEnv()
    if cmtprojectpath is None:
        if env.has_key("CMTPROJECTPATH") :
            del env["CMTPROJECTPATH"]
        if cmtpath is None :
            if env.has_key("CMTPATH") :
                del env["CMTPATH"]
        else :
            env["CMTPATH"] = cmtpath
    else :
        env["CMTPROJECTPATH"] = cmtprojectpath
        if env.has_key("CMTPATH"):
            del env["CMTPATH"]



def doesDirMatchVersion(directory, version, casesense=True):
    if not version :
        found = True
    else :
        if not casesense :
            directory = directory.upper()
            version = version.upper()
        dirlist = directory.split(os.sep)
        found = False
        if dirlist[-1] == version :
            found = True
    return found

def doesDirMatchName(directory, name, casesense=True):
    if not name :
        found = True
    else :
        if not casesense :
            directory = directory.upper()
            name = name.upper()
        dirlist = directory.split(os.sep)
        namelist = name.split(os.sep)
        if len(namelist) == 1 and name.find("/") != -1 :
            namelist = name.split("/")
        found = True
        for k in range(len(namelist)) :
            if namelist[-1-k] != dirlist[-1-k] :
                found = False
                break
        # with version directory
        if not found :
            found = True
            for k in range(len(namelist)) :
                if namelist[-1-k] != dirlist[-2-k] :
                    found = False
                    break
    return found

def doesDirMatchNameAndVersion(directory, name , version, casesense=True):
    found = False
    if doesDirMatchName(directory, name, casesense ) :
        if version is None :
            found = True
        elif doesDirMatchVersion(directory, version, casesense):
            found = True
    return found


def isDirSelected(directory, select, casesense=True):
    if select is None :
        return True
    if not casesense:
        directory = directory.upper()
        select = select.upper()
    if directory.find(select) != -1 :
        return True
    else :
        return False


def addCMTTag(tag, environ):
    if environ.has_key("CMTEXTRATAGS") :
        taglist = environ["CMTEXTRATAGS"].split(",")
        if tag not in taglist :
            taglist.append(tag)
            environ["CMTEXTRATAGS"] = ",".join(taglist)
    else :
        environ["CMTEXTRATAGS"] = tag

def isCMTMessage(line, extra=None):
    ismsg = False
    msg_prefixes = []
    if os.environ.has_key("CMTMSGPREFIX") :
        cpref = os.environ["CMTMSGPREFIX"]
        if cpref :
            msg_prefixes.append(cpref)
    msg_prefixes.append("#CMT--->")
    msg_prefixes.append("#CMT>")
    for p in msg_prefixes :
        s = p
        if extra :
            s += extra
        if line.find(s) != -1 :
            ismsg = True
            break
    return ismsg

def isCMTWarning(line):
    return isCMTMessage(line, " Warning:")

def CMTLog(msg):
    log = logging.getLogger()
    if isCMTWarning(msg) and ( msg.find("not found") != -1 or msg.find("Structuring style") != -1 ) :
        log.debug(msg)
    else :
        log.warning(msg)

def CMTCommand(*args, **kwargs) :
    oripwd = None
    if "PWD" in os.environ.keys() :
        oripwd = os.environ["PWD"]
    if "cwd" in kwargs.keys() :
        os.environ["PWD"] = kwargs["cwd"]

    out, err, code = apply(callCommand, ("cmt",) + args, kwargs)

    if oripwd :
        os.environ["PWD"] = oripwd
    else :
        del os.environ["PWD"]

    return out, err, code


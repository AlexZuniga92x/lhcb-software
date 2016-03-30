import ROOT
import re
import types
import veloview.analysis
from ..config import Config
from .combiners import RootCombiner
from .interface import ComparisonFunction, ValueFunction
from ..runview.utils import run_file, reference_run_file

class AnalysisConfigWrapper(object):
    """
    This class reads the analysis config file into a class structure of
    Combiners.
    """
    TELL1_NUMBERS_R   = range(42)
    TELL1_NUMBERS_PHI = range(64, 106)
    TELL1_NUMBERS_ALL = TELL1_NUMBERS_R + TELL1_NUMBERS_PHI

    def __init__(self, config):
        """
        Initialise an AnalysisConfigWrapper from a configuration.

        @type  config: 2-tuple of dictionaries
        @param config: First element is dictionary of branches, second element
                       is dictionary of leaves.
        """
        self.masterBranch = "MasterCombiner"
        self.branches = config[0]
        self.leaves = config[1]

    def getTrunkForRun(self, runNr, limitRegExp = None):
        """
        Get a Combiner object that represents the trunk of the tree.

        @type  runNr:        int
        @param runNr:        Run number.
        @type  limitRegExp:  string
        @param limitRegExp:  Regular expression to which a leaf name must match
                             for it to be included in the tree, or None to
                             match any leaf name. Default None.
        @return              A Combiner object.
        """
        dataFilePath = run_file(runNr)
        refFilePath = reference_run_file(runNr)
        return self.getTrunk(dataFilePath, refFilePath, limitRegExp, runNr)

    def getTrunk(self, dataFilePath, refFilePath, limitRegExp = None, runNr = 0):
        """
        Get a Combiner object that represents the trunk of the tree.

        @type  dataFilePath: string
        @param dataFilePath: Path to the file containing the data that the
                             Combiner should analyse.
        @type  refFilePath:  string
        @param refFilePath:  Path to the file containing reference data, to
                             which the Combiner compares the actual data.
        @type  limitRegExp:  string
        @param limitRegExp:  Regular expression to which a leaf name must match
                             for it to be included in the tree, or None to
                             match any leaf name. Default None.
        @return              A Combiner object.
        """
        comb_dict = {}
        eval_dict = {}

        limit = re.compile(limitRegExp) if limitRegExp else None

        for leafName, leafDict in self.leaves.iteritems():
            if limit and not limit.match(leafName):
                continue

            parsedLeaf = self._parseLeaf(leafName, leafDict)
            for leafComb, leafEval in parsedLeaf:
                newName = leafComb["name"]
                comb_dict[newName] = leafComb
                eval_dict[newName] = leafEval

        for branchName, branchDict in self.branches.iteritems():
            branchComb = self._parseBranch(branchDict)
            comb_dict[branchName] = branchComb

        return RootCombiner(comb_dict, eval_dict, dataFilePath, refFilePath, runNr)

    def _parseLeaf(self, leafName, leafDict):
        """
        Returns a list of tuples of the form (name, combDictEntry,
        evalDictEntry). combDictEntry is an entry that can be added as a value
        to the dictionary that's supplied to the "desc_dict" argument of
        Combiner's constructor. evalDictEntry is an entry that can be added as
        a value to the dictionary that's supplied to the "eval_dict" argument
        of Combiner's constructor. name is the key with which this entry can be
        added to the dictionaries. The content for these values is taken from
        the argument. Acts as a conversion between front-end Combiner
        formatting (analysis_config.py) and formatting read by the Combiner
        class.

        @type  leafDict: dictionary
        @param leafDict: Leaf description dictionary, containing at least the
                         key "name" and "function" and one or more of the keys
                         "errThreshold", "warnThreshold", "params",
                         "sensor_dependent", and "trending".
        @return          A list of tuples.
        """
        path       = leafDict["name"]
        maxError   = leafDict.get("errThreshold",  50)
        maxWarning = leafDict.get("warnThreshold", 80)
        function   = self.__findAnalysisClass(leafDict["comparison"])
        arg        = leafDict.get("params", None)
        sensor_dep = leafDict.get("sensor_dependent", False)
        trending = []
        for trend in leafDict.get("trending", []):
            if isinstance(trend, tuple):
                trending.append((self.__findAnalysisClass(trend[0]),) + trend[1:])
            else:
                trending.append(self.__findAnalysisClass(trend))

        results = []
        if sensor_dep:
            children = []
            for tell1 in self.TELL1_NUMBERS_ALL:
                name   = "{}_{:03d}".format(leafName, tell1)
                myPath = path.format(tell1)
                results.append((\
                    {\
                        "name":       name      ,\
                        #Tell1 child nodes all have same weight
                        "weight":     1.        ,\
                        "path":       myPath    ,\
                        "maxError":   maxError  ,\
                        "maxWarning": maxWarning,\
                    },\
                    {\
                        "Function":   function  ,\
                        "Argument":   arg       ,\
                        "trending":   trending  ,\
                    }\
                ))
                children.append(name)

            # Add master branch containing all the above leaves
            masterBranch = self._parseBranch(leafDict)
            masterBranch["name"]     = leafName
            masterBranch["children"] = children
            results.append((masterBranch, {}))
        else:
            # Normal leaf
            results.append((\
                {\
                    "name":       leafName                  ,\
                    "weight":     leafDict.get("weight", 1.),\
                    "path":       path                      ,\
                    "maxError":   maxError                  ,\
                    "maxWarning": maxWarning                ,\
                },\
                {\
                    "Function":   function                  ,\
                    "Argument":   arg                       ,\
                    "trending":   trending                  ,\
                }\
            ))

        return results

    def _parseBranch(self, branchDict):
        """
        Return an entry that can be added as a value to the dictionary that's
        supplied to the "desc_dict" argument of Combiner's contructor. The
        content of the entry is taken from the argument. Acts as a conversion
        between front-end Combiner formatting (analysis_config.py) and
        formatting read by the Combiner class.

        @type  branchDict: dictionary
        @param branchDict: Branch description dictionary, containing one or
                           more of the keys "weight", "children", "minWW",
                           "minWE", "minEW", "minEE"
        """
        return\
            {\
                "weight":   branchDict.get("weight",   1.),\
                "children": branchDict.get("children", []),\
                "minWW":    branchDict.get("minWW",    5 ),\
                "minWE":    branchDict.get("minWE",   10 ),\
                "minEW":    branchDict.get("minEW",    1 ),\
                "minEE":    branchDict.get("minEE",    2 ),\
            }

    @classmethod
    def __findAnalysisClass(cls, className):
        """
        Find a class in the package "veloview.analysis". Throws an exception if
        the class doesn't exist, or if it doesn't inherit from
        ComparisonFunction.

        @type  className: string
        @param className: The name of the class to find.
        """
        if not isinstance(className, str):
            raise TypeError("Argument className should be a string")
        if not className in dir(veloview.analysis):
            raise ValueError("Class {} not found in \"veloview.analysis\"".format(className))
        value = getattr(veloview.analysis, className)
        if not type(value) is types.TypeType:
            raise ValueError("Class {} not found in \"veloview.analysis\", found a {} instead".format(className, type(value)))
        if not (issubclass(value, ComparisonFunction) or issubclass(value, ValueFunction)):
            raise ValueError("Class {} should inherit from ComparisonFunction or ValueFunction".format(className))
        return value


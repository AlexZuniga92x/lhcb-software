"""Methods dealing with run view plots."""
import glob

import ROOT

from veloview.config import Config
from veloview.config.run_view import run_view_pages
from veloview.core import io
from veloview.giantrootfile.gui_tree import Tree
from veloview.runview import utils
from veloview.runview.response_formatters import dictionary_formatter

def get_run_plot(name, run, reference=False, formatter=dictionary_formatter, 
                 refRun = None, normalise = False, notifyBox = None):
    """Return the formatted object at the plot path in the run file.

    If reference is True, the corresponding plot from the reference file will
    be returned, else the plot from the `run` run will be returned.
    If no reference run can be found for `run`, None is returned.
    Keyword arguments:
    name -- Path within the run file to the plot object.
            A KeyError is raised if name is not found in the run file
    run -- Run number
    reference -- If True, include the reference plot for the given plot and run
    formatter -- Response formatter used to format the TObject
    """
    if reference and refRun==None:
        try:
            run = utils.reference_run(name, run)
        except ValueError:
            return None

    elif reference: run = refRun

    # Get the latest run file in the run's directory
    try:
        path = utils.run_file(run)
    except IOError:
        if notifyBox != None:
            notifyBox.notify("Run file not found for run {0}".format(run), 'run file not found')
        raise IOError("Run file not found for run {0}".format(run))

    # Try to open the file
    f = ROOT.TFile(path)
    if f.IsZombie():
        if notifyBox != None:
            notifyBox.notify("Run file is zombie for run {0}".format(run), 'is zombie')
        raise IOError("Run file is zombie for run {0}".format(run), 'is zombie')

    # Retrieve the object
    obj = f.Get(name)
    if not obj:
        if notifyBox != None:
            msg = "Plot {0} not found in run file {1}"
            msg = msg.format(name, run).rstrip()
            notifyBox.notify("Plot {0} not found in run file {1}".format(name, run), 'not found')
        raise KeyError("Plot {0} not found in run file {1}".format(name, run), 'not found')
    # The file will be closed when the function returns, so we need to clone
    # the fetched object outside the file's scope
    ROOT.gROOT.cd()
    clone = obj.Clone(obj.GetName())
    f.Close()

    # Normalise histogram if required
#     plot_dict = get_plot_dictionary(name)
#     if plot_dict is not None and plot_dict.get('normalised', False): normalise = True
    if normalise:
        integral = clone.Integral()
        if integral > 0:
            clone.Scale(1.0/integral)

    return formatter(clone)
        
        
def get_run_plot_with_reference(name, run, formatter=dictionary_formatter, 
                                refRun = None, normalise = False, notifyBox = None):
    """Return the formatted nominal and reference plots.

    A 2-tuple of two plots is returned:
        1. The nominal plot, as returned by
            get_run_plot(name, run, reference=False, formatter)
        2. The reference plot, as returned by
            get_run_plot(name, run, reference=True, formatter)
    in that order.

    If the reference get_run_plot call returns None, None is returned in place
    of the reference object.
    """
    nominal = get_run_plot(name, run, reference=False, formatter=formatter, 
                           normalise = normalise, notifyBox = notifyBox)
    try:
        reference = get_run_plot(name, run, reference=True,
                                 formatter=formatter, refRun = refRun, 
                                 normalise= normalise, notifyBox = notifyBox)
    except KeyError:
        reference = None
    return nominal, reference


def get_plot_dictionary(name):
  for page in run_view_pages.itervalues():
    if 'plots' not in page:
        continue
    for plot in page['plots']:
      if plot['name'] == name:
        return plot

def format_run_range(runRange):
    if len(runRange) == 2:
        return lambda t: t.runnr >= runRange[0] and t.runnr <= runRange[1]
    return lambda t: t.runnr in runRange

def get_trending_plot(name, runRange, formatter = dictionary_formatter):
    """
    Get a trending plot, showing a certain variable plotted against run number.
    @param name the name of the variable to plot.
    @param runRange list of all run numbers to plot. If this contains exactly
           two items, it is treated as a range instead, plotting all runs with
           run number greater than or equal to the first item but not greater
           than the second.
    """
    f = ROOT.TFile(Config().grf_file_path, 'READ')
    t = Tree(Config().grf_tree_name)
    data = t.Data( 
                ( # functions projecting out coordinates to plot
                    lambda t: t.runnr,
                    lambda t: getattr(t, name).value(),
                ),
                ( # cut(s) to apply
                    format_run_range(runRange),
                )
            )

    formatted = formatter(dict(name=name, title='run number versus {0}'.format(name), xLabel="run number", yLabel=name, data=data))

    del t
    f.Close()
    del f

    return formatted

def get_2d_trending_plot(nameX, nameY, runRange, formatter = dictionary_formatter):
    """
    Get a trending plot, showing two variables plotted against each other.
    @param nameX the name of the first variable to plot.
    @param nameY the name of the second variable to plot.
    @param runRange list of all run numbers to plot. If this contains exactly
           two items, it is treated as a range instead, plotting all runs with
           run number greater than or equal to the first item but not greater
           than the second.
    """
    f = ROOT.TFile(Config().grf_file_path, 'READ')
    t = Tree(Config().grf_tree_name)
    data = t.Data( 
                ( # functions projecting out coordinates to plot
                    lambda t: getattr(t, nameX).value(),
                    lambda t: getattr(t, nameY).value(),
                ),
                ( # cut(s) to apply
                    format_run_range(runRange),
                )
            )

    formatted = formatter(dict(name='{0};{1}'.format(nameX, nameY), title='{0} versus {1}'.format(nameX, nameY), xLabel=nameX, yLabel=nameY, data=data))

    del t
    f.Close()
    del f

    return formatted


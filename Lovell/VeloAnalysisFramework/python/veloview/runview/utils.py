"""Utilities for GUI run view pages."""
from veloview.config import Config
from veloview.runview.reference_database import ReferenceDatabase
from veloview.utils import paths, rundb


def run_list():
    """Return a list of run numbers as integers sorted high-to-low."""
    run_file_lines = tuple(open(Config().processed_run_list_path, "r"))
    return sorted([int(l.strip()) for l in run_file_lines], reverse=True)


def valid_run(run):
    """Return True if run is a run number present in the run number file.

    Run numbers not present do not have corresponding run files and so are
    considered invalid.
    """
    return run in run_list()


def sensor_list():
    """Return a list of sensor numbers as integers."""
    return range(0, 47) + range(64, 107)


def valid_sensor(sensor):
    """Return True if sensor is a valid sensor number."""
    return sensor in sensor_list()


def reference_run(plot, run):
    """Return the reference run number for the plot and nominal run number."""
    refdb = ReferenceDatabase(Config().reference_db_path)
    polarity = rundb.RunDB().polarity(run)
    return refdb.reference_run_for_plot(run, plot, polarity)


def run_file_path(run):
    """Return TFile object directory path for the given run."""
    return paths.make_dir_tree(run, Config().run_data_dir)

#!/usr/bin/env python
"""Return a plot for a given run in the format expected by the offline GUI.

A run number is in invalid if it is not present in the run list.
A plot name is invalid if it is not present in the run file.
A sensor number is invalid if is not in {0, ..., 42, 64, ..., 106}.
If any invalid arguments are supplied, 1 is returned and the program exits.
"""
# This is due to what appears to be a ROOT bug, where doing `import ROOT`
# prints a control sequence to stdout on xterm terminal types
import os
os.environ["TERM"] = "dumb"

import sys
import argparse


def create_parser():
    from veloview.core import config as veloview_config
    parser = argparse.ArgumentParser(
        description=__doc__.split("\n")[0],
        epilog="\n".join(__doc__.split("\n")[1:]))
    parser.add_argument("run", type=int, help="Run number")
    parser.add_argument("plot", type=str, help="Plot name in run file")
    parser.add_argument("sensor", type=int, nargs="?", default=0,
                        help="Sensor number")
    parser.add_argument("--run-data-dir", default=veloview_config.run_data_dir,
                        help="Directory to search for run list and data")
    parser.add_argument("--reference", action="store_true",
                        help="Include reference plot for the given arguments")
    return parser


def exit_with_error(msg):
    """Print the failure JSON response with msg and return exit code 1."""
    import json
    sys.stdout.write(json.dumps({'success': False, 'message': msg}))
    sys.exit(1)


def retrieve_run_view_plot(run, plot, sensor, reference, run_data_dir):
    from veloview.core import config as veloview_config
    from veloview.runview import plots, response_formatters, utils

    # Change the run data directory to the user-specified one
    veloview_config.update_run_data_dir(run_data_dir)

    # Check all arguments have valid values
    err = False
    if not utils.valid_run(run):
        err = True
        msg = "Invalid run number provided: {0}".format(run)
    if not utils.valid_sensor(sensor):
        err = True
        msg = "Invalid sensor number provided: {0}".format(sensor)
    if err:
        exit_with_error(msg)

    # Format the plot name with the sensor number
    # str.format will work even with no format specifiers in the string
    plot = plot.format(sensor)

    # Try to the get the plot object, formatting it to JSON
    try:
        response = plots.get_run_plot(plot, run, reference=reference,
                                      formatter=response_formatters.json_formatter)
    except KeyError, e:
        err = True
        msg = (
            "Invalid plot name provided: {0}. "
            "Exception caught: {1}."
        ).format(plot, e)
    except TypeError, e:
        err = True
        msg = (
            "Cannot handle plot type for plot: {0}. "
            "Exception caught: {1}."
        ).format(plot, e)
    except Exception, e:
        # We finally catch every, to make sure clients have an error message to
        # show
        err = True
        msg = (
            "An unidentified exception was raised fetching plot: {0}. "
            "Please contact the maintainer with the information."
            "Exception: {1}"
        ).format(plot, e)

    if err:
        exit_with_error(msg)

    sys.stdout.write(response)


def main():
    parser = create_parser()
    args = parser.parse_args()
    retrieve_run_view_plot(args.run, args.plot, args.sensor, args.reference,
        args.run_data_dir)


if __name__ == "__main__":
    main()
    sys.exit(0)

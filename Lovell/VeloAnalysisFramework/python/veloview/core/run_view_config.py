"""GUI run view configuration.

The ordered dictionary run_view_pages defined in this file is used by both GUIs
to discover what pages to create, what plots to show on each page, and how to
display them.

Each page is defined by a key in the top level of run_view_pages. The `title`
key must be defined as a string of the name to show for the page. An optional
`plots` key can define a list of dictionaries, each defining a plot within
a Vetra output file. An example page dictionary might look like this:

    {
        'title': 'Foo Things',
        'plots': [
            {
                'title': 'Plot A',
                'name': 'folder/sensor_{0:03d}/plot_name_A',
                'sensor_dependent': True
            },
            {
                'title': 'Plot B',
                'name': 'folder/subfolder/plot_name_B'
            },
        ]
    }

For each plot dictionary, the following keys can be defined:

    title: Title to display (required, string)
    name: Path within the Vetra output file to the plot If sensor_dependent is
          True, should contain a format placeholder defining how the sensor
          number is formatted (required, string)
    sensor_dependent: If True, there is one plot per VELO sensor, and the
                      `name` key defines at least one formatting placeholder
                      for the sensor number (boolean)
    short: A shorter title to display, useful if `title` is very long and might
           look unwieldy in a GUI tab (string)
    options: Dictionary of options defining purely cosmetic attributes
             (dictionary)

The format placeholder that must be present in `name` if `sensor_dependent` is
True must follow the specification outlined in the Python format
mini-language [1]. In essence, a format placeholder is a pair of braces `{}`
containing an integer followed by a optional colon and formatting defintions.
The integer represents the index of the list of arguments passed to the
call to `string.format`, and the formatting defintions specify how the value
of that argument will be formatted in to a string. In the example above,
`format` will be called on the value of `name`, being passed the sensor number
as the first argument.

    >>> 'folder/sensor_{0:03d}/plot_name_A'.format(5)
    'folder/sensor_005/plot_name_A'

No key in the `options` dictionary is required, and valid keys are:

    showUncertainties: Should the GUI display the uncertainties defined in the
                       plots (boolean)

If there is a cosmetic change you would like to be adjustable for each plot
that is not already available, please contact the authors of the GUIs.

[1]: https://docs.python.org/2/library/string.html#formatspec
"""
from collections import OrderedDict

run_view_pages = OrderedDict([
    ('dqs', {
        'title': 'DQS'
    }),
    ('pedestals', {
        'title': 'Pedestals',
        'plots': [
            {
                'title': 'Pedestal bank',
                'name': 'Vetra/VeloPedestalSubtractorMoni/TELL1_{0:03d}/Pedestal_Bank',
                'sensor_dependent': True
            },
            {
                'title': 'Subtracted ADC profile',
                'name': 'Vetra/VeloPedestalSubtractorMoni/TELL1_{0:03d}/Ped_Sub_ADCs_Profile',
                'sensor_dependent': True
            },
            {
                'title': 'Subtracted ADC 2D',
                'name': 'Vetra/VeloPedestalSubtractorMoni/TELL1_{0:03d}/Ped_Sub_ADCs_2D',
                'sensor_dependent': True
            }
        ]
    }),
    ('common_mode', {
        'title': 'Common mode'
    }),
    ('noise', {
        'title': 'Noise',
        'plots': [
            {
                'title': 'RMS noise vs. chip channel',
                'name': 'Vetra/NoiseMon/DecodedADC/TELL1_{0:03d}/RMSNoise_vs_ChipChannel',
                'sensor_dependent': True
            },
            {
                'title': 'RMS noise vs. strip',
                'name': 'Vetra/NoiseMon/DecodedADC/TELL1_{0:03d}/RMSNoise_vs_Strip',
                'sensor_dependent': True
            },
        ]
    }),
    ('clusters', {
        'title': 'Clusters',
        'plots': [
            {
                'title': 'Number of VELO clusters per event (Default)',
                'short': 'Clusters per event',
                'name': 'Velo/VeloClusterMonitor/# VELO clusters'
            },
            {
                'title': 'Number of strips per cluster',
                'short': 'Strips per cluster',
                'name': 'Velo/VeloClusterMonitor/Cluster size',
                'options': {
                    'showUncertainties': True
                }
            },
            {
                'title': 'Active chip links versus sensor',
                'short': 'Active links per sensor',
                'name': 'Velo/VeloClusterMonitor/Active chip links vs sensor'
            },
            {
                'title': 'Number of strips per cluster versus sensor',
                'short': 'Strips per cluster vs. sensor',
                'name': 'Velo/VeloClusterMonitor/Cluster size vs sensor'
            }
        ]
    }),
    ('occupancy', {
        'title': 'Occupancy',
        'plots': [
            {
                'title': 'Channel occupancy',
                'name': 'Velo/VeloOccupancyMonitor/OccPerChannelSens{0}',
                'sensor_dependent': True
            },
            {
                'title': 'Average sensor occupancy',
                'name': 'Velo/VeloOccupancyMonitor/OccAvrgSens'
            },
            {
                'title': 'Occupancy spectrum (zoom)',
                'short': 'Occupancy spectrum',
                'name': 'Velo/VeloOccupancyMonitor/OccSpectMaxLow'
            },
            {
                'title': '% VELO occupancy vs. LHC bunch ID (A side)',
                'short': 'Occupancy vs. BCID (A side)',
                'name': 'Velo/VeloOccupancyMonitor/h_veloOccVsBunchId_ASide'
            },
            {
                'title': '% VELO occupancy vs. LHC bunch ID (C side)',
                'short': 'Occupancy vs. BCID (C side)',
                'name': 'Velo/VeloOccupancyMonitor/h_veloOccVsBunchId_CSide'
            }
        ]
    }),
    ('tracks', {
        'title': 'Tracks'
    }),
    ('vertices', {
        'title': 'Vertices'
    }),
    ('errors', {
        'title': 'Errors',
        'plots': [
            {
                'title': 'Errors per sensor',
                'name': 'Velo/ErrorMon/Error Counter'
            }
        ]
    }),
    ('sensor_overview', {
        'title': 'Sensor overview'
    })
])

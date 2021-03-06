#!/usr/bin/env python3

"""Create bias input files from calculated pmfs"""

import numpy as np
import json
from origamipy import us_process


def main():
    filebase = 'inps/snodin-staple_temp-344_run-0_rep-0' # simualation identification name
    wins_filename = 'inps/snodin_344_staple.windows' # windows file
    tags, wins = us_process.read_windows_file(wins_filename) # returns [tag1, tag2, ..,] and [((min1, max1), (min2, max2), ...)]
    pmfs_filename = 'outs/snodin-long_temp-344_run-1_rep-0_pmfs.sds'
    pmf_array = np.loadtxt(pmfs_filename, skiprows=1)
    pmfs = {(i[0], i[1]): i[2] for i in pmf_array} # dictionary for each line{(col1, col2):col3}
    for win in wins:
        biases = {'biases':[]}
        domain_lims = [win[0][0], win[1][0]]
        staple_lims = [win[0][1], win[1][1]]
        for domain in range(domain_lims[0], domain_lims[1] + 1):
            for staple in range(staple_lims[0], staple_lims[1] + 1):
                point = (staple, domain)
                if point not in pmfs:
                    continue
                elif np.isnan(pmfs[point]):
                    continue
                else:
                    biases_entry = {}
                    biases_entry['point'] = [domain, staple]
                    biases_entry['bias'] = -pmfs[point]
                    biases['biases'].append(biases_entry)

        win_filename = us_process.create_win_filename(win, filebase)
        json.dump(biases, open(win_filename, 'w'), indent=4,
                separators=(',', ': '))


if __name__ == '__main__':
    main()

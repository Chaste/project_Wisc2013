#!/usr/bin/env python

"""
Generate alternative versions of the crypt proliferation protocol result plots for publication.

These use the data generated for the automatic plots, but adjust titles, labels, etc. to fit
with the context in the paper, and add a key (see ticket 1999 for work on automating this part).
"""

import os
import sys

# The test output folder may be supplied as an argument; if not the current folder is assumed
if len(sys.argv) > 1:
    os.chdir(sys.argv[1])

models = ['UniformWnt', 'VariableWnt', 'StochasticGenerationBased', 'ContactInhibition']
model_names = ['Uniform Wnt', 'Variable Wnt', 'Stochastic Generation-based', 'Contact Inhibition']
plots = ['Cell_age_distribution', 'Cell_division_locations']

copy_templates = {'model%d/%s.eps': '%s-%s_auto.eps',
                  'model%d/outputs_%s_gnuplot_data.csv': '%s-%s_data.csv'}

plot_template = r"""# Gnuplot script for crypt cell division study
set terminal postscript eps enhanced size 4,3 font 16
set output "%(eps)s"
set title '%(title)s'
set xlabel 'Percentage height up the crypt (%%)'
set ylabel '%(ylabel)s'
set grid
set autoscale
set key outside invert title "Crypt height"
set datafile separator ","
plot "%(csv)s" using 1:2 title "10" with linespoints pointtype 7,\
     "%(csv)s" using 1:3 title "12" with linespoints pointtype 7,\
     "%(csv)s" using 1:4 title "14" with linespoints pointtype 7,\
     "%(csv)s" using 1:5 title "16" with linespoints pointtype 7,\
     "%(csv)s" using 1:6 title "18" with linespoints pointtype 7,\
     "%(csv)s" using 1:7 title "20" with linespoints pointtype 7,\
     "%(csv)s" using 1:8 title "22" with linespoints pointtype 7,\
     "%(csv)s" using 1:9 title "24" with linespoints pointtype 7,\
     "%(csv)s" using 1:10 title "26" with linespoints pointtype 7,\
     "%(csv)s" using 1:11 title "28" with linespoints pointtype 7,\
     "%(csv)s" using 1:12 title "30" with linespoints pointtype 7
"""

#plot_titles = ['Age distribution', 'Cell division locations']
plot_ylabels = ['Mean cell age at division (hours)', 'Number of divisions per box']
def plot_title(plot_index, model_index):
    if plot_index == 0:
        return 'Cell age distribution'
    else:
        return '%c) %s' % ('abcd'[model_index], model_names[model_index])

for i, model in enumerate(models):
    for j, plot in enumerate(plots):
        for src_tpl, dest_tpl in copy_templates.items():
            src = src_tpl % (i, plot)
            dest = dest_tpl % (model, plot)
            os.system('cp "%s" "%s"' % (src, dest))
        # Write a specialised gnuplot script, rather than the FC default, and run it
        subst = {'csv': '%s-%s_data.csv' % (model, plot),
                 'eps': '%s-%s.eps' % (model, plot),
                 'title': plot_title(j, i),
                 'ylabel': plot_ylabels[j]}
        script_name = '%s-%s_script.gp' % (model, plot)
        script = open(script_name, 'w')
        script.write(plot_template % subst)
        script.close()
        os.system('gnuplot "%s"' % script_name)

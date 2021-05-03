import os

import zenutils


this_dir = os.path.dirname(os.path.abspath(__file__))
pyd_lib_dir = os.path.join(this_dir, 'pydlib')
dso_lib_dir = os.path.join(this_dir, 'dsolib')


def import_library(name):
    return zenutils.import_library(pyd_lib_dir, name)


for name in os.listdir(dso_lib_dir):
    if name.endswith('.so'):
        zenutils.load_library(os.path.join(dso_lib_dir, name))

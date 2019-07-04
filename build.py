#!/usr/bin/python

# Important: import panda3d as the very first library - otherwise it crashes
import panda3d.core  # noqa

import sys
import os
import argparse
from os.path import join, realpath, dirname

# Change into the current directory
os.chdir(dirname(realpath(__file__)))

from scripts.common import get_ini_conf, write_ini_conf  # noqa
from scripts.setup import make_output_dir, run_cmake, run_cmake_build

if __name__ == "__main__":

    # Arguments
    parser = argparse.ArgumentParser(description="P3DModuleBuilder")
    parser.add_argument(
        '--optimize', type=int, default=None,
        help="Optimize level, should match the one used for the Panda3D build",)
    parser.add_argument(
        "--clean", action="store_true", help="Forces a clean rebuild")
    args = parser.parse_args()

    # Python 2 compatibility
    if sys.version_info.major > 2:
        raw_input = input

    config_file = join(dirname(realpath(__file__)), "config.ini")
    config = get_ini_conf(config_file)

    # Find cached module name
    if "module_name" not in config or not config["module_name"]:
        module_name = str(raw_input("Enter a module name: "))
        config["module_name"] = module_name.strip()

    # Check for outdated parameters
    for outdated_param in ["vc_version", "use_lib_eigen", "use_lib_bullet", "use_lib_freetype"]:
        if outdated_param in config:
            print("WARNING: Removing obsolete parameter '" + outdated_param + "', is now auto-detected.")
            del config[outdated_param]

    # Write back config
    write_ini_conf(config, config_file)

    # Just execute the build script
    make_output_dir(clean=args.clean)
    run_cmake(config, args)
    run_cmake_build(config, args)

    print("Success!")
    sys.exit(0)

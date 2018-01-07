#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

import sys
import argparse
import os
sys.path.append('TestFramework')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--command-line',
                        action='store_true',
                        help='Use command line version instead of GUI')
    parser.add_argument('-s', '--suite',
                        help='Specify test suite to load')

    opts = parser.parse_args(sys.argv[1:])
    print opts
    if opts.command_line:
        import CmdApp
        app = CmdApp.App()
    else:
        import GuiApp
        app = GuiApp.App()
    app.run(os.path.join('TestFramework', 'TestSuites'),
            'TestPal.xml',
            os.path.join('TestFramework', 'TestCases'))

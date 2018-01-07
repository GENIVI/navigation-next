#!/usr/bin/env python
# -*- coding: utf-8 -*-
from App import AppBase


class App(AppBase):
    """Command line App.
    """

    def __init__(self):
        """
        Arguments:
        - `suite`:
        """
        AppBase.__init__(self)

    def run(self, suiteDir=None, suite=None, casesDir=None):
        """
        """
        print("Running suite: %s\n"%(suite))
        print("OK....\n")

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob
from Utility import *
import xml.etree.ElementTree as ET
import copy

class TestCase(object):
    """
    """

    def __init__(self, name, enabled=False):
        """

        Arguments:
        - `name`:
        - `enabled`:
        """
        self._name    = name
        self._enabled = enabled

    def GetName(self):
        """
        """
        return self._name

    def IsEnabled(self):
        """
        """
        return self._enabled

    def __str__(self):
        """
        """
        return 'TestCase:(Name: %s, enabled: %d)'%(self._name,  self._enabled)

class TestSuite(object):
    """Test Suite, a collection of test _cases.
    For the sake of simplicity, only enabled _cases are saved in XML files.
    """

    def __init__(self, path, cases):
        """
        """
        self._path     = path
        self._allCases = cases; # list of all _cases.
        self._name     = "Default"
        self._cases    = {}
        self._loadFromFile()
        pass

    def Save(self):
        """
        """
        root = ET.Element('TestSuite', {'name':self._name})
        for case in self._cases:
            if case.IsEnabled():
                e = ET.SubElement('item', {'name':case.GetName()})

        root.write(self._path, encoding="utf-8")

    def GetName(self):
        """
        """
        return self._name;

    def GetTestCases(self):
        """
        """
        return self._cases.values()

    def _loadFromFile(self):
        """Load test suite from path.
        """
        if not os.path.isfile(self._path):
            print "Can't find file: %s"%self._path
            return

        # use a simple ElementTree to parse XML.
        tree = ET.parse(self._path)
        root = tree.getroot()
        self._name = root.attrib.get('name', 'Default')
        i = root.iter('item')
        while True:
            try:
                element = i.next()
            except StopIteration as e:
                break
            caseName = element.attrib.get('name')
            self._cases[caseName] = TestCase(caseName, True)

        keys = self._cases.keys()
        for case in self._allCases:
            caseName = case.GetName()
            if not caseName in keys:
                 #todo: check if shallow copy works..
                self._cases[caseName] = copy.copy(case)

class TestSuiteProvider(object):
    """This provider scans all stored suites.
    """
    def __init__(self):
        """
        """
        self._suiteDir = None
        self._casesDir  = None
        self._testSuites = {}
        self._allCases = []

    def SetCasesDir(self, casesDir):
        self._casesDir = casesDir
        self._loadCases()

    def SetSuiteDir(self, suiteDir):
        self._suiteDir = suiteDir
        #todo: collect all xml files

    def GetTestSuite(self, suiteName):
        """
        Retrieve named test suite.
        Arguments:
        - `suiteName`: name of test suite to be retrieved.
        """
        return self._testSuites.get('tmp')

    def LoadTestSuite(self, filename):
        """Load specified testSuite from filename.
        """
        suite = TestSuite(os.path.join(self._suiteDir, filename), self._allCases)
        return suite

    def _loadCases(self):
        """Load all test _cases.
        """
        self._cases = set()
        if not os.path.isdir(self._casesDir):
            return

        caseFiles = glob.glob(os.path.join(self._casesDir,"Test*.h"))
        for path in caseFiles:
            self._allCases.append(TestCase(filename_sans_extention(path)))

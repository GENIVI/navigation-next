#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import wx
import wx.lib.filebrowsebutton as filebrowse
import wx.lib.agw.customtreectrl as CT

sys.path.append('TestFramework/UI')
sys.path.append('TestFramework/Data')
from App import AppBase
from TestSuiteProvider import *
from TestExecutor import *


## common things

class LargeBoldFont(wx.Font):
    """
    """

    def __init__(self):
        wx.Font.__init__(self, 18, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD)

class IdGeneratorFactory(object):
    """Factory to generate IdGenerator.
    Every widget should use this factory to retrieve a IdGenerator and use
    retrieved IdGenerator to generate Ids for smaller widgets.
    """

    def __init__(self):
        """
        """
        self._step   = 100;
        self._baseId = 10000

    def __GenerateBaseId(self):
        """
        """
        tmp = self._baseId;
        self._baseId += self._step;
        return tmp;

    def GetIdGenerator(self):
        """
        """
        class CompononentIdGenerator(object):
            """
            """
            def __init__(self, baseId):
                """

                Arguments:
                - `baseId`:
                """
                self._nextId = baseId

            def GenerateID(self):
                """
                """
                tmpId = self._nextId
                self._nextId += 1
                return tmpId;

        return CompononentIdGenerator(self.__GenerateBaseId())


g_idGenerator = IdGeneratorFactory()

class CaseTreeCtrl(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent)
        self.InitUI()
        pass

    def InitUI(self):
        """
        """
        self.style = (CT.TR_DEFAULT_STYLE|CT.TR_MULTIPLE
                      |CT.TR_FULL_ROW_HIGHLIGHT|CT.TR_AUTO_CHECK_CHILD
                      |CT.TR_AUTO_CHECK_PARENT|CT.TR_AUTO_TOGGLE_CHILD)
        self.tree = CT.CustomTreeCtrl(self, agwStyle=self.style)

        self.treeRoot = self.tree.AddRoot("TestSuite: Not loaded..")
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.tree, 1, wx.EXPAND)
        self.SetSizer(sizer)

    def ShowTestSuite(self, suite):
        """
        """
        if not suite:
            return
        self.suite = suite
        self.treeRoot.SetText("Test Suite:" + self.suite.GetName())
        for case in suite.GetTestCases():
            print case
            iNode = self.tree.AppendItem(self.treeRoot, case.GetName(), ct_type=1)
            self.tree.CheckItem(iNode, case.IsEnabled())
        self.tree.Expand(self.treeRoot)
        pass

    def GetEnabledCases(self):
        """
        """
        cases = []
        (item, cookie) = self.tree.GetFirstChild(self.treeRoot)
        while item:
            if self.tree.IsItemChecked(item):
                cases.append(TestCase(self.tree.GetItemText(item), True))
                print cases[-1]
            (item, cookie)= self.tree.GetNextChild(self.treeRoot, cookie)

        if self.suite.GetName == 'Default':
            #todo: Add dialog to save this suite.
            # we only need to save enabled ones, disabled ones can be added dynamically.
            pass

        return cases


class CasesPanel(wx.Panel):
    """Panel to operate test cases.
    """

    def __init__(self, parent):
        """Initializer.

        Arguments:
        - `parent`:
        """
        wx.Panel.__init__(self, parent)
        self._parent = parent
        self._suiteProvider = TestSuiteProvider()
        self._idGenerator = g_idGenerator.GetIdGenerator()
        self.InitUI()
        pass

    def InitUI(self):
        """
        """
        sizer = wx.BoxSizer(wx.VERTICAL)

        # load and display name of test suite.
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        fbb = filebrowse.FileBrowseButton(self, -1, size=(450, -1),
                                          changeCallback = self.fbbCallback)
        fbb.SetLabel("TestSuite")
        hbox.Add(fbb)
        sizer.Add(hbox)

        # to display test cases in a suite.
        self.tree = CaseTreeCtrl(self)
        self.tree.Bind(CT.EVT_TREE_ITEM_CHECKED, self.OnItemChecked)
        sizer.Add(self.tree, 1, wx.EXPAND)

        # start/stop/compile...
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        self.id_start = self._idGenerator.GenerateID()
        self.startBtn = wx.Button(self, self.id_start, "Start")
        self.startBtn.Bind(wx.EVT_BUTTON, self.OnButtonPressed)
        hbox.Add(self.startBtn, 0, wx.EXPAND|wx.ALL, 3)

        self.id_stop = self._idGenerator.GenerateID()
	self.stopBtn = wx.Button(self, self.id_stop, "Stop")
        self.stopBtn.Bind(wx.EVT_BUTTON, self.OnButtonPressed)
        self.stopBtn.Enable(False)
        hbox.Add(self.stopBtn, 0, wx.EXPAND|wx.ALL, 3)

        sizer.Add(hbox)

        self.SetBackgroundColour("pink")
        self.SetSizer(sizer);

        pass

    def OnButtonPressed(self, evt):
        btn = evt.GetEventObject()
        id = evt.GetId()
        if id == self.id_start:
            self.startBtn.Enable(False)
            self.stopBtn.Enable(True)
            self._StartTest()
        elif id == self.id_stop:
            self.stopBtn.Enable(False)
            self.startBtn.Enable(True)
            self._StopTest()

    def fbbCallback(self, evt):
        print('FileBrowseButton: %s\n' % evt.GetString())

    def SetTestSuite(self, suiteDir, suite, casesDir):
        """
        """
        self._suiteProvider.SetSuiteDir(suiteDir)
        self._suiteProvider.SetCasesDir(casesDir)
        self.testSuite = self._suiteProvider.LoadTestSuite(suite)
        self.tree.ShowTestSuite(self.testSuite)

    def SetExecuter(self, executer):
        """

        Arguments:

        - `executer`:
        """
        self.executer = executer
        self.executer.SetCallback(self)

    def OnFinish(self):
        """
        """
        self.startBtn.Enable(True)
        self.stopBtn.Enable(False)

    def OnItemChecked(self, evt):
        """Handler when an item is checked.
        Arguments:
        - `evt`:
        """
        #todo: provide a way to save test suite.
        # self._saveBtn.Enable(True)
        evt.Skip()
        pass

    def _StartTest(self):
        """
        """
        cases = self.tree.GetEnabledCases()
        if self.executer:
            self.executer.SetTestCases(cases)
            self.executer.Begin()
        pass


    def _StopTest(self):
        """
        """
        if self.executer:
            self.executer.Stop()
            pass


class MySplitter(wx.SplitterWindow):
    def __init__(self, parent, ID):
        wx.SplitterWindow.__init__(self, parent, ID,
                                   style = wx.SP_LIVE_UPDATE)

        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGED, self.OnSashChanged)
        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGING, self.OnSashChanging)

    def OnSashChanged(self, evt):
        pass

    def OnSashChanging(self, evt):
        pass
    # uncomment this to not allow the change
    #evt.SetSashPosition(-1)

myEVT_TEXT_UPDATED = wx.NewEventType()
EVT_TEXT_UPDATED = wx.PyEventBinder(myEVT_TEXT_UPDATED, 1)
class CountEvent(wx.PyCommandEvent):
    """Event to signal that a count value is ready"""
    def __init__(self, etype, eid, value=None):
        """Creates the event object"""
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

class OutputPanel(wx.Panel):
    """
    """

    def __init__(self, parent):
        """
        """
        wx.Panel.__init__(self, parent, -1)
        self._idGenerator = g_idGenerator.GetIdGenerator()
        self.InitUI()

    def InitUI(self):
        """
        """
        sizer = wx.BoxSizer(wx.VERTICAL);
        label = wx.StaticText(self, -1, "Output", (5,5))
        label.SetFont(LargeBoldFont())
        sizer.Add(label)

        self.txt = wx.TextCtrl(self, -1, style=wx.TE_MULTILINE|wx.TE_READONLY)
        sizer.Add(self.txt, 1, wx.EXPAND)

        hbox = wx.BoxSizer(wx.HORIZONTAL)

        self._id_clear = self._idGenerator.GenerateID()
        self._clear_btn = wx.Button(self, self._id_clear, "Clear")
        hbox.Add(self._clear_btn, wx.EXPAND|wx.ALL|wx.ALIGN_RIGHT, 3)

        self._id_save = self._idGenerator.GenerateID()
        self._save_btn = wx.Button(self, self._id_save, "Save")
        hbox.Add(self._save_btn, 0, wx.EXPAND|wx.ALL|wx.ALIGN_RIGHT, 3)


        self.Bind(wx.EVT_BUTTON, self.OnButtonPressed)
        self.Bind(EVT_TEXT_UPDATED, self.OnTextUpdated)

        sizer.Add(hbox, 0, wx.ALIGN_RIGHT)
        self.SetSizer(sizer)

    def GetLogger(self):
        """
        """
        return self

    def OnTextUpdated(self, evt):
        """
        @todo: parse log files and highlight texts.

        Arguments:

        - `evt`:
        """
        self.txt.AppendText(evt.GetValue()+"\n")

    def OnButtonPressed(self, evt):
        """

        Arguments:

        - `evt`:
        """
        id = evt.GetId()
        if id == self._id_clear:
            self.txt.Clear()


    def WriteLog(self, fmt, *args):
        """
        Arguments:
        - `self`:
        - `fmt`:
        - `...`:
        """
        try:
            if args:
                msg = fmt % args
            else:
                msg = fmt
        except :
            msg = "Not formatted.."
        evt = CountEvent(myEVT_TEXT_UPDATED, -1, msg)
        wx.PostEvent(self, evt)

class MainFrame(wx.Frame):
    """MainFrame of test framework.
    """

    def __init__(self):
        """
        """
        wx.Frame.__init__(self, None, title="CCC Test Framework", size=(1024, 768))

        self.InitUI()
        self.Centre()

        self.executer = TestExecutor(self.outputPanel.GetLogger())
        self.casePanel.SetExecuter(self.executer)

    def InitUI(self):
        sizer = wx.BoxSizer(wx.VERTICAL)

        splitter = MySplitter(self, -1)

        self.casePanel = CasesPanel(splitter)

        self.outputPanel = OutputPanel(splitter)
        self.outputPanel.SetBackgroundColour("sky blue")

        splitter.SplitVertically(self.casePanel, self.outputPanel)
        splitter.SetMinimumPaneSize(450)

        sizer.Add(splitter, 1, wx.EXPAND)
        self.SetSizer(sizer)

    def SetTestSuite(self, suiteDir, suite, casesDir):
        """
        """
        self.casePanel.SetTestSuite(suiteDir, suite, casesDir)


class App(AppBase):
    """Command line App.
    """

    def __init__(self):
        """
        Arguments:
        - `suite`:
        """
        AppBase.__init__(self)
        self.InitUI()

    def InitUI(self):
        self.app = wx.App()
        self.frame = MainFrame();
        self.frame.Show(True)


    def run(self, suiteDir=None, suite=None, casesDir=None):
        """
        """
        if not suite:
            suite='TestPal.xml'

        print("Running suite: %s\n"%(suite))
        self.frame.SetTestSuite(suiteDir, suite, casesDir)
        self.app.MainLoop()
        print("OK....\n")

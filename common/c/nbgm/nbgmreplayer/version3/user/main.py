from userscript import *
import sys,imp,os,logging,unittest

CURRENT_PATH = os.path.split(os.path.realpath(__file__))[0]

def test_case(folder_name, module_name = 'case', entry_point_func_name = 'Run'):
    abs_path = os.path.join(CURRENT_PATH, folder_name)      
    sys.path.append(abs_path)
    fp, pathname, description = imp.find_module(module_name)
    user_module = imp.load_module(module_name, fp, pathname, description)
    user_module.__dict__[entry_point_func_name]()
    sys.path.pop()
    
def test_cases(folder_name_list, module_name = 'case', entry_point_func_name = 'Run'):
    for folder in folder_name_list:
        test_cast(folder, module_name, entry_point_func_name)

def Run():
   # add your custom case 
    test_case('case1')
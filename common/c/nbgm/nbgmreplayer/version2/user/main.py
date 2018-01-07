from userscript import *
import sys,imp,os,logging,unittest

CURRENT_PATH = os.path.split(os.path.realpath(__file__))[0]

def run_case(folder_name, module_name , entry_point_func_name):
    abs_path = os.path.join(CURRENT_PATH, folder_name)      
    sys.path.append(abs_path)
    fp, pathname, description = imp.find_module(module_name)
    user_module = imp.load_module(module_name, fp, pathname, description)
    user_module.__dict__[entry_point_func_name]()
    sys.path.pop()
    
def run_cases(folder_name_list, module_name, entry_point_func_name):
    for folder in folder_name_list:
        run_case(folder, module_name, entry_point_func_name)

def Run():
   # add your custom case
   run_case('demo_case', 'case', 'Run')
   #run_case('mobius_cases', 'mobius', 'Run')
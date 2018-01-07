"""
  Android.mk generator.
"""
import os, os.path
import sys

P4_ROOT = '../../../../../../..'
PROJ_ROOT = '../..'
BRANCH = os.environ['NIM_CCC_BRANCH']
BUILD_ROOT = os.path.join(P4_ROOT, 'client/c/core/build', BRANCH)

sys.path.insert(0, os.path.join(BUILD_ROOT, 'util'))
import ndk

project_path = ndk.ProjectPath(P4_ROOT, PROJ_ROOT, BRANCH)

nbpal_inc = [
    'nbpal_inc',
    'nbpal_inc/private',
    'nbpal_inc/posix',
    'nbpal_inc/android_ndk',
    'nbpal_src/android_ndk',
    'nbpal_src/posix/timer',
    'nbpal_src/android_ndk/pal',
    'nbpal_src/common/database',
    'nbpal_src/posix/net',
    'nbpal_src/android_ndk/taskqueue',
    'nbpal_src/common/taskqueue',
    'nbpal_src/posix/taskqueue',
    'nbpal_src/android_ndk/util',
    'nbpal_src/common',
    'nbpal_src/common/finishfunction',
    'nbpal_src/common/unzip',
    'nbpal_inc/../../../thirdparty/feature_111107_map3d/sqlite/include'
]

nbpal_src = {
    'include' : ['nbpal_src/android_ndk', 'nbpal_src/common', 'nbpal_src/util'],
    'exclude' : [],
    }

components = {
    'MY_NBPAL' : {'inc': nbpal_inc, 'src': nbpal_src, 'module_name' : 'nbpal', 'gen_version' : True, 'need_junction' : True, 'path_prefix' : 'nbpal_src'},
}

def gen_mk():
    generator = ndk.Generator(project_path, components)
    generator.gen()

def print_dirs():
    print ('P4_ROOT : %s' % os.path.abspath(os.path.join(__file__, '..', P4_ROOT)))
    print ('PROJ_ROOT : %s' % os.path.abspath(os.path.join(__file__, '..', PROJ_ROOT)))
    print ('BRANCH : %s' % BRANCH)
    print ('BUILD_ROOT : %s' % BUILD_ROOT)
    print ('project_path.root(): %s' % project_path.root())
    print ('project_path.jni(): %s' % project_path.jni())
    print ('nbm : %s' % project_path.nbm())

def main():
    print_dirs()
    gen_mk()

if __name__=='__main__':
    main()

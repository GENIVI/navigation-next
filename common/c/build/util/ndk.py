"""
 Android.mk generation.
"""

import os, os.path, stat
import sys, subprocess
import fnmatch
import string
import copy
import shutil

#NIM_PERFORCE_PATH = os.environ['NIM_PERFORCE_PATH']
#NIM_CCC_BRANCH = os.environ['NIM_CCC_BRANCH']

#PROJ_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
#PROJ_JNI = os.path.join(PROJ_ROOT, 'jni')

#def ccc(rel):
#    return string.replace(rel, '/release_141013_ltk_v1_5', '/' + NIM_CCC_BRANCH + '/')

class ProjectPath:
    def __init__(self, p4_root, proj_root, branch):
        self._p4_root = p4_root
        self._root = proj_root
        self._jni = os.path.join(self._root, 'projects/android_ndk/jni')
        self._branch = branch
        self._nbm = None

    def ccc(self, rel_path):
        rel_path = string.replace(rel_path, '/release_141013_ltk_v1_5', '/' + self._branch)
        return os.path.join(self._p4_root, 'client/c/core', rel_path)

    def ccc_keep_path(self, rel_path):
        return os.path.join(self._p4_root, 'client/c/core', rel_path)

    def py_version(self, dir_name):
        return os.path.join(self._p4_root, 'client/c/core', dir_name, self._branch, 'build/version.py')

    def build(self):
        return self.ccc('build/release_141013_ltk_v1_5')

    def py_util_build_android(self):
        return os.path.join(self.build(), 'util/build_android.py')

    def root(self):
        return self._root

    def p4_root(self):
        return self._p4_root

    def jni(self):
        return self._jni

    def branch(self):
        return self._branch

    def nbm(self):
        if self._nbm is not None:
            return self._nbm
        filename = os.path.join(self.build(), 'nbm_android_version.txt')
        try:
            f = open(filename, "r")
            ver = f.read()
            folder = "nbm_" + ver + "_android"
            #self._nbm = os.path.join(self._p4_root, 'core/releases/nbm/android', folder)
            self._nbm = os.path.join(self._p4_root, 'client/c/core/thirdparty', self.branch(), 'nbm/android', folder)
        except IOError:
            self._nbm = ''
        return self._nbm

    def dir(self, name):
        return os.path.join(self.ccc(name), self.branch())

#
# Global components setting
#   inc: array of strings; include directories; optional, default is []
#   src: dictionary; list of directories whose files are recursively included; optional, default is { 'include':[], 'exclude':[]}
#   module_name: string; output module name; required
#   shared_libs: array of strings; local shared library dependency; optional, default is []
#   static_libs: array of strings; local static library dependency; optional, default is []
#   is_static: boolean; True if component is static; optional, default is False
#   gen_version: boolean; True if component requires version header generation; optional, default is False
#   gen_tpslib: boolean; True if component requires tpslib protocol code generation; optional, default is False
#   dir_name: string; directory name of the component; optional, default is module_name
#   is_prebuilt: string; if component is prebuilt; optional, default is False
#   need_junction: boolean; True if component requires symbolic links to src in jni directory; optional, default is false
#   path_prefix: string; If a file path has the prefix, the prefix is removed from the path before write to Android.mk; optional, default is None
#
#components = {
#    'MY_NBPAL' : {'inc': nbpal_inc, 'src': nbpal_src, 'module_name' : 'nbpal', 'gen_version' : True},
#    'MY_ABPAL' : {'inc' : abpal_inc, 'src' : abpal_src, 'module_name' : 'abpal', 'gen_version' : True},
#    'MY_CORESERVICES' : {'inc' : coreservices_inc, 'src' : coreservices_src, 'module_name' : 'coreservices', 'shared_libs' : ['nbpal'], 'gen_version': True },
#    'MY_NBSERVICES' : {'inc' : nbsvcs_i, 'src' : nbsvcs_s, 'module_name' : 'nbservices', 'shared_libs' : ['nbpal', 'coreservices', 'abpal', 'nbgm'], 'static_libs' : ['nbdata', 'nbnav', 'nbprotocol', 'nbm'], 'gen_version' : True, 'gen_tpslib': True },
#    'MY_NBNAV'     : { 'src' : nbnav_src, 'module_name' : 'nbnav', 'is_static' : True},
#    'MY_NBDATA'     : {'inc' : nbdata_inc, 'src' : nbdata_src, 'module_name' : 'nbdata', 'is_static' : True},
#    'MY_NBPROTOCOL' : {'inc' : nbprotocol_inc, 'src' : nbprotocol_src, 'module_name' : 'nbprotocol', 'is_static' : True},
#    'MY_ABSERVICES' : {'inc' : abservices_inc, 'src' : abservices_src, 'module_name' : 'abservices', 'shared_libs' : ['coreservices', 'nbservices', 'abpal', 'nbpal'], 'static_libs' : ['nbdata'], 'gen_version' : True},
#    'MY_JNI' : {'inc' : jni_inc, 'src' : jni_src, 'module_name' : 'jni', 'shared_libs' : ['nbservices', 'coreservices', 'abpal', 'nbpal', 'abservices'], 'static_libs' : ['nbnav']},
#    'MY_NBGM' : {'inc' : nbgm_inc, 'src' : nbgm_src, 'module_name' : 'nbgm', 'shared_libs' : ['nbpal','coreservices'], 'gen_version' : True},
#    'MY_NBM': {'src' : nbm_src, 'is_static' : True, 'is_prebuilt' : True, 'module_name' : 'nbm'}
#}

class Component:
    def __init__(self, name, data):
        self.data = data
        self._name = name
    def name(self):
        return self._name
    def inc(self):
        try:
            return self.data['inc']
        except KeyError:
            return []
    def src(self):
        try:
            return self.data['src']
        except KeyError:
            return {'include':[], 'exclude': []}
    def module_name(self):
        return self.data['module_name']
    def shared_libs(self):
        try:
            return self.data['shared_libs']
        except KeyError:
            return []
    def static_libs(self):
        try:
            return self.data['static_libs']
        except KeyError:
            return []
    def is_static(self):
        try:
            return self.data['is_static']
        except KeyError:
            return False
    def gen_version(self):
        try:
            return self.data['gen_version']
        except KeyError:
            return False
    def gen_tpslib(self):
        try:
            return self.data['gen_tpslib']
        except KeyError:
            return False
    def dir_name(self):
        try:
            return self.data['dir_name']
        except KeyError:
            return self.module_name()
    def is_prebuilt(self):
        try:
            return self.data['is_prebuilt']
        except KeyError:
            return False
    def need_junction(self):
        try:
            return self.data['need_junction']
        except KeyError:
            return False
    def path_prefix(self):
        try:
            return self.data['path_prefix']
        except KeyError:
            return None


class MkFileGen:
    def __init__(self, project_path):
        self._project_path = project_path
        file = os.path.join(self._project_path.root(), 'projects/android_ndk/jni/Android.mk')
        if os.path.exists(file):
            os.chmod(file, stat.S_IRWXG | stat.S_IRWXO | stat.S_IRWXU)
        self.file = open(file, 'w')

    def prefix(self):
        s = """
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

"""
        self.file.write(s)

    def posfix(self, modules):
        s = """
include $(CLEAR_VARS)
LOCAL_MODULE := ccc
LOCAL_WHOLE_STATIC_LIBRARIES = staticlibs
LOCAL_LDLIBS := -lGLESv1_CM  -llog
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
include $(BUILD_SHARED_LIBRARY)
"""
        #self.file.write(s.replace('staticlibs', string.join(modules, ' ')))

    def inc(self, name, prefix, dirs):
        self.file.write("\n")
        #print 'name : %s' % name
        #print 'prefix : %s' % prefix
        #print 'dirs: %s' % dirs
        list = []
        for dir in dirs:
            if sys.platform == 'win32':
                dir = dir.replace('\\', '/')
            if (not prefix is None) and (dir.find(prefix) != -1):
                dir = dir.replace(prefix, '$(LOCAL_PATH)/../../../')
            else:
                dir = '$(LOCAL_PATH)/../' + dir  # from jni directory
            list.append(dir)
        self.__gen_continue_lines(name, list)

    def incs(self, list):
        self.__gen_plus_lines('MY_C_INCLUDES', list)

    def srcs(self, list):
        self.__gen_plus_lines('LOCAL_SRC_FILES', list)

    def src(self, prefix, dirmap):
        files = []
        for file in self.__all_files(dirmap['include'], dirmap['exclude']):
            #print file
            fname, fExtension = os.path.splitext(file)
            ext = fExtension.lower()
            if ext in ('.c', '.cpp', '.cxx', '.a', '.so'):
                if sys.platform == 'win32':
                    file = file.replace('\\', '/')
                if (not prefix is None) and (file.find(prefix) != -1):
                    file = file.replace(prefix, '')
                else:
                    file = os.path.join('..', file) # from jni directory
                files.append(file)
        self.__gen_continue_lines('LOCAL_SRC_FILES', files)

    def src_pre(self, component, is_prebuilt):
        s = """
include $(CLEAR_VARS)
LOCAL_MODULE := component
"""
        self.file.write(s.replace('component', component))
        if not is_prebuilt:
            s2 ="""LOCAL_CFLAGS := -DANDROID_NDK -DNBGM_IPHONE_SDK -DDEBUG_LOG_DEBUG
LOCAL_LDLIBS := -lGLESv1_CM -llog -lz
LOCAL_C_INCLUDES := $(MY_C_INCLUDES)
"""
            self.file.write(s2)

    def src_post(self, is_static, is_prebuilt, shared_libs, static_libs):
        if len(shared_libs):
            self.file.write('\n')
            self.file.write('LOCAL_SHARED_LIBRARIES := ' + string.join(shared_libs, ' '))
        if len(static_libs):
            self.file.write('\n')
            self.file.write('LOCAL_STATIC_LIBRARIES := ' + string.join(static_libs, ' '))
        if is_static:
            if is_prebuilt:
                s = "include $(PREBUILT_STATIC_LIBRARY)"
            else:
                s = "include $(BUILD_STATIC_LIBRARY)"
        else:
            if is_prebuilt:
                s = "include $(PREBUILT_SHARED_LIBRARY)"
            else:
                s = "include $(BUILD_SHARED_LIBRARY)"
        self.file.write('\n' + s + '\n')

    def __gen_continue_lines(self, name, lines):
        for i in range(len(lines)):
            s = lines[i]
            if sys.platform == 'win32':
                s = s.replace('\\', '/')
            if i == 0:
                self.file.write(name + ' := ' + s)
            else:
                self.file.write(' \\\n\t' + s)

    def __gen_plus_lines(self, name, list):
        self.file.write('\n')
        for i in range(len(list)):
            if i == 0:
                op = ' := '
            else:
                op = ' += '
            self.file.write(name + op + '$(' + list[i] + ')\n')

    def __gen_include_static_lib(self, module):
        self.file.write('\n')
        self.file.write('include $(BUILD_STATIC_LIBRARY)\n')

    def __gen_local_module(self, module):
        self.file.write('\n')
        self.file.write('LOCAL_MODULE := ' + module)

    def __all_files(self, roots, exclude_patterns='*', single_level=False, yield_folders=False):
        # Expand patterns from semicolon-separated string to list
        for root in roots:
            if os.path.isfile(root):
                yield root
                continue
            for path, subdirs, files in os.walk(root):
                if yield_folders:
                    files.extend(subdirs)
                files.sort( )
                for name in files:
                    include = True
                    full_name = os.path.join(path, name)
                    #print 'root: %s' % root
                    #print 'full_name: %s' % full_name
                    for pattern in exclude_patterns:
                        #print 'pattern: %s' % pattern
                        if fnmatch.fnmatch(full_name, pattern):
                            include = False
                            break
                    if include:
                        yield full_name
                if single_level:
                    break

class MkFile:
    def __init__(self, project_path):
        self.modules = []
        self.components = []
        self._project_path = project_path

    def add_component(self, component):
        self.components.append(component)

    def gen_version(self):
        for c in self.components:
            if c.gen_version():
                subprocess.call(['python', self._project_path.py_version(c.dir_name()), self._project_path.dir(c.dir_name()), 'android_ndk'])

    def gen_tpslib(self):
        for c in self.components:
            if c.gen_tpslib():
                subprocess.call(['python', self._project_path.py_util_build_android(), self._project_path.build()], cwd = self._project_path.build())


    def gen(self):
        mkgen = MkFileGen(self._project_path);
        mkgen.prefix()
        for c in self.components:
            mkgen.inc(c.name(), c.path_prefix(), c.inc())
        mkgen.incs([c.name() for c in self.components])
        for c in self.components:
            mkgen.src_pre(c.module_name(), c.is_prebuilt())
            mkgen.src(c.path_prefix(), c.src())
            mkgen.src_post(c.is_static(), c.is_prebuilt(), c.shared_libs(), c.static_libs())
            #mkgen.srcs(self.srcmap.keys())
        mkgen.posfix(self.modules)

ccc_dirmap = {
    'nbpal_src' : 'nbpal/release_141013_ltk_v1_5/src',
    'nbpal_inc' : 'nbpal/release_141013_ltk_v1_5/include',
    'nbpal_lib' : 'nbpal/release_141013_ltk_v1_5/projects/android_ndk/libs/armeabi',
    'abpal_src' : 'abpal/release_141013_ltk_v1_5/src',
    'abpal_inc' : 'abpal/release_141013_ltk_v1_5/include',
    'abpal_lib' : 'abpal/release_141013_ltk_v1_5/projects/android_ndk/libs/armeabi',
    'coreservices_src' : 'coreservices/release_141013_ltk_v1_5/src',
    'coreservices_inc' : 'coreservices/release_141013_ltk_v1_5/include',
    'coreservices_lib' : 'coreservices/release_141013_ltk_v1_5/projects/android_ndk/libs/armeabi',
    'nbsvcs_s' : 'nbservices/release_141013_ltk_v1_5/src',
    'nbsvcs_i' : 'nbservices/release_141013_ltk_v1_5/include',
    'nbsvcs_l' : 'nbservices/release_141013_ltk_v1_5/projects/android_ndk/obj/local/armeabi',
    'abservices_src' : 'abservices/release_141013_ltk_v1_5/src',
    'abservices_inc' : 'abservices/release_141013_ltk_v1_5/include',
    'abservices_lib' : 'abservices/release_141013_ltk_v1_5/projects/android_ndk/libs/armeabi',
    'nbgm_src' : 'nbgm/release_141013_ltk_v1_5/src',
    'nbgm_inc' : 'nbgm/release_141013_ltk_v1_5/include',
    'nbgm_lib' : 'nbgm/release_141013_ltk_v1_5/projects/android_ndk/libs/armeabi',
    'jni_src' : 'jni/release_141013_ltk_v1_5/src',
    'jni_inc' : 'jni/release_141013_ltk_v1_5/include',
    'nbui_src' : 'nbui/release_141013_ltk_v1_5/src',
    'nbui_inc' : 'nbui/release_141013_ltk_v1_5/include',
    #'nbm' : project_path.nbm(),
    }

class Dirmap:
    def __init__(self, project_path, dirmap):
        self._dirmap = dirmap
        self._project_path = project_path

    def expand(self, s):
        if s is None:
            return s
        key = s.split('/')[0]
        try:
           value  = self._dirmap[key]
           result = s.replace(key, self._project_path.ccc(value))
#           print (os.path.abspath(result))
        except KeyError:
            if key == "nbm":
                result = s.replace(key, self._project_path.nbm())
            else:
                result = s

        # need to do match with the prefix of a file
        # which use '/'
        if sys.platform == 'win32':
            result = result.replace('\\', '/')

        return result


class Generator:
    def __init__(self, project_path, components):
        self._components = components
        self._project_path = project_path
        self._dirmap = Dirmap(self._project_path, ccc_dirmap)

    def gen(self):
        mkfile = MkFile(self._project_path)
        for key in self._components.keys():
            value = self.__expand(key, self._components[key])
            component = Component(key, value)
            if component.need_junction():
                self.__junction_add()
                self.__copy_build_android('Application.mk', self._project_path.jni())
                self.__copy_build_android('project.properties', '.')
            mkfile.add_component(component)
        mkfile.gen()

    def __expand(self, name, component):
        c = Component(name, component)
        inc = [self._dirmap.expand(p) for p in c.inc()]
        src_include = [self._dirmap.expand(p) for p in c.src()['include']]
        src_exclude = [self._dirmap.expand(p) for p in c.src()['exclude']]
        path_prefix = self._dirmap.expand(c.path_prefix())
        if path_prefix is not None:
            path_prefix = os.path.dirname(path_prefix) + '/'
        cc = copy.deepcopy(component)
        cc['inc'] = inc
        cc['src']['include'] = src_include
        cc['src']['exclude'] = src_exclude
        cc['path_prefix'] = path_prefix
        return cc

    def __junction_add(self):
        j = os.path.join(self._project_path.jni(), 'src')
        f = os.path.join(os.path.abspath(self._project_path.root()), 'src')
        if not os.path.exists(j):
            if sys.platform == 'win32':
                subprocess.call(["junction.exe ", j , f])
            elif sys.platform == 'darwin':
                subprocess.call(["ln", "-s", f, j])
        print j, f

    def __copy_build_android(self, filename, dst_dir):
        src = os.path.join(self._project_path.build(), 'android', filename)
        dst = os.path.join(dst_dir, filename)
        if os.path.exists(dst):
            os.chmod(dst, stat.S_IRWXG | stat.S_IRWXO | stat.S_IRWXU)
        shutil.copy(src, dst)

def main():
    print project_path.root()


if __name__=='__main__':
    main()

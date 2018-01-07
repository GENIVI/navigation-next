import os.path
import sys, stat, subprocess
import fnmatch
import string
import optparse

SEPERATE_MAKE_FLAG = True

NIM_CCC_BRANCH   = 'feature_140523_hybrid_phase2'
CPU_TYPE         = os.environ['NIM_CCC_CPU_ARCH']
NBM_VERSION_FILE = 'nbm_android_version.txt'
NCDB_VERSION_FILE = 'ncdb_android_version.txt'

# Make junction paths as short as possible, in order to avoid "Argument list too long" issue.
# If this issue still exists, please use "subst" command or make these paths below shorter,
# or devide modules into samller ones.
#nbpal
NBPAL_SRC_PATH = 'nps'
NBPAL_INC_PATH = 'npi'
#abapl
ABPAL_SRC_PATH = 'aps'
ABPAL_INC_PATH = 'api'
#jni
JNI_SRC_PATH = 'jnis'
JNI_INC_PATH = 'jnii'
#nbui
NBUI_SRC_PATH = 'nbuis'
NBUI_INC_PATH = 'nbuii'
#nbservices
NBSERVICES_INC_PATH = 'ni'
NBSERVICES_SRC_PATH = 'ns'
#coreservices
CORESERVICES_INC_PATH = 'csi'
CORESERVICES_SRC_PATH = 'css'
#abservices
ABSERVICES_INC_PATH = 'asi'
ABSERVICES_SRC_PATH = 'ass'
#thirdparty
THIRDPARY_PATH = 'tpj'
#nbm
NBM_PATH = 'nbmj'
#ncdb
NCDB_PATH = 'ncdbj'
#nbgm
NBGM_MAIN_SRC_PATH = 'nbgms'
NBGM_MAIN_INC_PATH = 'nbgmi'
NBRE_SRC_PATH = 'nbres'
NBRE_INC_PATH = 'nbrei'
GLES_SRC_PATH = 'gls'
GLES_INC_PATH = 'gli'
GLES2_SRC_PATH = 'gl2s'
GLES2_INC_PATH = 'gl2i'

MAP_INC_PATH = 'mapi'
MAP_SRC_PATH = 'maps'

COMMON_INC_PATH = 'commoni'
COMMON_SRC_PATH = 'commons'

NAV_INC_PATH = 'navi'
NAV_SRC_PATH = 'navs'

ccc_dirmap = {
    NBPAL_SRC_PATH : 'nbpal/main/src',
    NBPAL_INC_PATH : 'nbpal/main/include',
    ABPAL_SRC_PATH : 'abpal/main/src',
    ABPAL_INC_PATH : 'abpal/main/include',
    NBGM_MAIN_SRC_PATH : 'nbgm/main/nbgmmain/src',
    NBGM_MAIN_INC_PATH : 'nbgm/main/nbgmmain/include',
    NBRE_SRC_PATH : 'nbgm/main/nbre/src',
    NBRE_INC_PATH : 'nbgm/main/nbre/include',
    GLES_SRC_PATH : 'nbgm/main/rendersystem/gles/src',
    GLES_INC_PATH : 'nbgm/main/rendersystem/gles/include',
    GLES2_SRC_PATH: 'nbgm/main/rendersystem/gles2/src',
    GLES2_INC_PATH: 'nbgm/main/rendersystem/gles2/include',
    CORESERVICES_SRC_PATH : 'coreservices/main/src',
    CORESERVICES_INC_PATH : 'coreservices/main/include',
    NBSERVICES_SRC_PATH : 'nbservices/main/src',
    NBSERVICES_INC_PATH : 'nbservices/main/include',
    ABSERVICES_SRC_PATH : 'abservices/main/src',
    ABSERVICES_INC_PATH : 'abservices/main/include',
    JNI_SRC_PATH : 'jni/main/src',
    JNI_INC_PATH : 'jni/main/include',
    NBUI_SRC_PATH : 'nbui/main/src',
    NBUI_INC_PATH : 'nbui/main/include',
    THIRDPARY_PATH : 'thirdparty/main',
    NBM_PATH : '',
    NCDB_PATH : '',
    MAP_INC_PATH : 'map/main/include',
    MAP_SRC_PATH : 'map/main/src',
    COMMON_INC_PATH : 'common/main/include',
    COMMON_SRC_PATH : 'common/main/src',
    NAV_INC_PATH : 'nav/main/include',
    NAV_SRC_PATH : 'nav/main/src'
}

class ProjectPath:
    def __init__(self, p4_root, proj_root, branch, app_common_branch):
        self._p4_root = p4_root
        self._root = proj_root
        self._jni = os.path.join(self._root, 'jni')
        self._branch = branch
        self._app_common_branch = app_common_branch
        self._nbm = None
        self._ncdb = None

    def ccc(self, rel_path):
        rel_path = string.replace(rel_path, '/main', '/' + self._branch)
        return os.path.join(self._p4_root, 'client/c/core', rel_path)

    def ccc_keep_path(self, rel_path):
        return os.path.join(self._p4_root, 'client/c/core', rel_path)

    def appcommon(self, rel_path):
        rel_path = string.replace(rel_path, '/main', '/' + self._app_common_branch + '/')
        return os.path.join(self._p4_root, 'client/c/app_common', rel_path)

    def py_version(self, dir_name, root_dir):
        return os.path.join(self._p4_root, 'client/c/core', dir_name, self._branch, root_dir, 'build/version.py')

    def build(self):
        return self.ccc('build/main')

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
        filename = os.path.join(self.build(), NBM_VERSION_FILE)
        try:
            f = open(filename, "r")
            ver = f.read()
            folder = "nbm_" + ver + "_android"
            self._nbm = os.path.join(self._p4_root, 'core/releases/nbm/android', folder)
        except IOError:
            self._nbm = ''
        return self._nbm

    def ncdb(self):
        if self._ncdb is not None:
            return self._ncdb
        filename = os.path.join(self.build(), NCDB_VERSION_FILE)
        try:
            f = open(filename, "r")
            ver = f.read()
            folder = "ncdb_" + ver + "_android"
            self._ncdb = os.path.join(self._p4_root, 'core/releases/ncdb/android', folder)
        except IOError:
            self._ncdb = ''
        return self._ncdb

    def prof(self):
        return os.path.join(self.build(), 'android', 'profiler', 'v3.1')

    def dir(self, name, root_dir):
        return os.path.join(self.ccc(name), self.branch(), root_dir)

    def print_dirs(self):
        print ('project_path.p4_root(): %s' % self.p4_root())
        print ('project_path.root(): %s' % self.root())
        print ('project_path.jni(): %s' % self.jni())
        print ('project_path.build(): %s' % self.build())
        print ('nbm : %s' % self.nbm())
        print ('ncdb : %s' % self.ncdb())


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
            elif key == "ncdb":
                result = s.replace(key, self._project_path.ncdb())
            else:
                result = s

        # need to do match with the prefix of a file
        # which use '/'
        if sys.platform == 'win32':
            result = result.replace('\\', '/')

        return result

jcc_dirmap = {
    'src_nbui' : 'nbui/' + NIM_CCC_BRANCH +'/android/src',
    }


nbpal_inc = [
    NBPAL_INC_PATH,
    NBPAL_INC_PATH+'/protected',
    NBPAL_INC_PATH+'/android_ndk',
    NBPAL_INC_PATH+'/posix',
    NBPAL_SRC_PATH+'/android_ndk',
    NBPAL_SRC_PATH+'/android_ndk/net',
    NBPAL_SRC_PATH+'/android_ndk/pal',
    NBPAL_SRC_PATH+'/android_ndk/taskqueue',
    NBPAL_SRC_PATH+'/android_ndk/util',
    NBPAL_SRC_PATH+'/android_ndk/imageblender',
    NBPAL_SRC_PATH+'/android_ndk/jni',
    NBPAL_SRC_PATH+'/android_ndk/synchronization',
    NBPAL_SRC_PATH+'/common',
    NBPAL_SRC_PATH+'/common/database',
    NBPAL_SRC_PATH+'/common/finishfunction',
    NBPAL_SRC_PATH+'/common/taskqueue',
    NBPAL_SRC_PATH+'/common/nbtaskqueue',
    NBPAL_SRC_PATH+'/posix/taskqueue',
    NBPAL_SRC_PATH+'/posix/util',
    NBPAL_SRC_PATH+'/common/unzip',
    NBPAL_SRC_PATH+'/common/posix',
    NBPAL_SRC_PATH+'/posix/net',
    NBPAL_SRC_PATH+'/posix/taskqueue',
    NBPAL_SRC_PATH+'/posix/timer',
    THIRDPARY_PATH+'/sqlite/include',
    THIRDPARY_PATH+'/freetype-2.4.4/include',
    THIRDPARY_PATH+'/freetype-2.4.4/include/freetype',
    THIRDPARY_PATH+'/lp150b15',
    THIRDPARY_PATH+'/libjpeg/include',
    THIRDPARY_PATH+'/libjpeg/include/android',
    THIRDPARY_PATH+'/libjpeg_turbo/include',
    THIRDPARY_PATH+'/speex/public',
    THIRDPARY_PATH+'/speex/android_ndk',
    THIRDPARY_PATH+'/speex/speex-1.2rc1/include',
    THIRDPARY_PATH+'/speex/speex-1.2rc1/include/speex',
    THIRDPARY_PATH+'/speex/speex-1.2rc1/libspeex',
    THIRDPARY_PATH+'/speex/libogg-1.3.0/include/ogg',
    THIRDPARY_PATH+'/speex/libogg-1.3.0/include',
    THIRDPARY_PATH+'/glues',
]

nbpal_private_inc = [
    NBPAL_INC_PATH+'/private',
]

nbpal_src = {
    'include' : [NBPAL_SRC_PATH+'/android_ndk',
                 NBPAL_SRC_PATH+'/common/database',
                 NBPAL_SRC_PATH+'/common/finishfunction',
                 NBPAL_SRC_PATH+'/common/gl',
                 NBPAL_SRC_PATH+'/common/posix',
                 NBPAL_SRC_PATH+'/common/unzip',
                 NBPAL_SRC_PATH+'/common/taskqueue',
                 NBPAL_SRC_PATH+'/common/nbtaskqueue',
                 NBPAL_SRC_PATH+'/posix/clock',
                 NBPAL_SRC_PATH+'/posix/file',
                 NBPAL_SRC_PATH+'/posix/net',
                 NBPAL_SRC_PATH+'/posix/synchronization',
                 NBPAL_SRC_PATH+'/posix/taskqueue',
                 NBPAL_SRC_PATH+'/posix/timer',
                 NBPAL_SRC_PATH+'/posix/pal',
                 NBPAL_SRC_PATH+'/util',
                 THIRDPARY_PATH+'/sqlite/Sources',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/autofit/autofit.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/bdf/bdf.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/cff/cff.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftbase.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftbitmap.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/cache/ftcache.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftfstype.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftgasp.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftglyph.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/gzip/ftgzip.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftinit.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/lzw/ftlzw.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftstroke.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftsystem.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/smooth/smooth.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftbbox.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftgxval.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftlcdfil.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftmm.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftotval.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftpatent.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftpfr.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftsynth.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/fttype1.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftwinfnt.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/base/ftxf86.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/pcf/pcf.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/pfr/pfr.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/psaux/psaux.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/pshinter/pshinter.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/psnames/psmodule.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/raster/raster.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/sfnt/sfnt.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/truetype/truetype.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/type1/type1.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/cid/type1cid.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/type42/type42.c',
                 THIRDPARY_PATH+'/freetype-2.4.4/src/winfonts/winfnt.c',
                 THIRDPARY_PATH+'/lp150b15',
                 THIRDPARY_PATH+'/libjpeg/src',
                 THIRDPARY_PATH+'/libjpeg_turbo/src',
                 THIRDPARY_PATH+'/speex/libogg-1.3.0/src',
                 THIRDPARY_PATH+'/speex/speex-1.2rc1/libspeex',
                 THIRDPARY_PATH+'/speex/private',
                 THIRDPARY_PATH+'/glues',
                 ],
    'exclude' : ['*/'+NBPAL_SRC_PATH+'/posix/util/*',
                 '*/'+NBPAL_SRC_PATH+'/android_ndk/pal/wm_global.cpp',
                 '*/'+NBPAL_SRC_PATH+'/posix/net/palnet.cpp',
                 '*/'+NBPAL_SRC_PATH+'/posix/taskqueue/taskmanagermultithreaded.cpp',
                 '*/'+NBPAL_SRC_PATH+'/posix/taskqueue/threadposix.cpp',
                 '*/'+NBPAL_SRC_PATH+'/android_ndk/myplaces/*',
                 '*/'+NBPAL_SRC_PATH+'/android_ndk/radio/com_navbuilder_pal_android_ndk_RadioInterface.cpp',
                 '*/'+THIRDPARY_PATH+'/lp150b15/pngtest.c',
                 '*/'+THIRDPARY_PATH+'/speex/speex-1.2rc1/libspeex/testfile/*']
}

abpal_inc = [
    ABPAL_INC_PATH,
    ABPAL_INC_PATH+'/android_ndk',
    ABPAL_INC_PATH+'/protected',
    NBSERVICES_INC_PATH,
    NBSERVICES_INC_PATH+'/android_ndk'
]

abpal_private_inc = [
    ABPAL_INC_PATH+'/private',
    ABPAL_SRC_PATH+'/android_ndk'
]

abpal_src = {
    'include' : [
        ABPAL_SRC_PATH+'/android_ndk',
        ABPAL_SRC_PATH+'/util'
        ],
    'exclude' : [],
}

coreservices_inc = [
    CORESERVICES_INC_PATH,
    CORESERVICES_INC_PATH+'/protected',
    CORESERVICES_INC_PATH+'/android_ndk',
]

coreservices_private_inc = [
    CORESERVICES_INC_PATH+'/private',
]

coreservices_src = {
    'include' : [
        CORESERVICES_SRC_PATH
        ],

    'exclude' : [
        '*/'+CORESERVICES_SRC_PATH+'/unittests/*',
        '*/'+CORESERVICES_SRC_PATH+'/tps/tests/*'
    ]
}

nbdata_inc = [
    NBSERVICES_INC_PATH,
    NBSERVICES_INC_PATH+'/protected',
    NBSERVICES_INC_PATH+'/android_ndk',
]

nbdata_private_inc = [
    NBSERVICES_INC_PATH+'/private',
]

nbdata_src = {
    'include' : [NBSERVICES_SRC_PATH+'/data'],
    'exclude' : [
        NBSERVICES_SRC_PATH+'/data/data_retrieve_confirmed.c'
    ]
}

nbprotocol_inc = [
    NBSERVICES_INC_PATH+'/generated'
]

nbprotocol_private_inc = [
    NBSERVICES_INC_PATH+'/generated/private'
]

nbprotocol_src = {
    'include' : [
        NBSERVICES_SRC_PATH+'/protocol'
    ],
    'exclude' : [
    ]
}

nbservices_inc = [
    NBSERVICES_INC_PATH,
    NBSERVICES_INC_PATH+'/protected',
    NBSERVICES_INC_PATH+'/android_ndk',
    NBSERVICES_INC_PATH+'/generated',
    NBSERVICES_SRC_PATH+'/map/layermanager',
    NBSERVICES_SRC_PATH+'/map/unified',
    NBSERVICES_SRC_PATH+'/map/mapview',
    NBSERVICES_SRC_PATH+'/map/tilemanager',
    NBSERVICES_SRC_PATH+'/map/tempmobius',
    NBSERVICES_SRC_PATH+'/analytics',
    NBUI_INC_PATH,
    NBUI_INC_PATH+'/android_ndk',
    NBUI_INC_PATH+'/protected',
    THIRDPARY_PATH+'/tinyxml',
    THIRDPARY_PATH+'/libxml2',
    THIRDPARY_PATH+'/libxml2/include',
    THIRDPARY_PATH+'/libxml2/include/platforms/android',
]

nbservices_private_inc = [
    NBSERVICES_INC_PATH+'/private',
    NBSERVICES_INC_PATH+'/generated/private',
]

nbservices_src = {
    'include' : [NBSERVICES_SRC_PATH+'/analytics',
                 NBSERVICES_SRC_PATH+'/enhancedcontent',
                 NBSERVICES_SRC_PATH+'/geocode',
                 NBSERVICES_SRC_PATH+'/gps',
                 NBSERVICES_SRC_PATH+'/map',
                 NBSERVICES_SRC_PATH+'/nav',
                 NBSERVICES_SRC_PATH+'/nbcommon',
                 NBSERVICES_SRC_PATH+'/network',
                 NBSERVICES_SRC_PATH+'/poiimagemanager',
                 NBSERVICES_SRC_PATH+'/proxpoi',
                 NBSERVICES_SRC_PATH+'/qalog',
                 NBSERVICES_SRC_PATH+'/rastertile',
                 NBSERVICES_SRC_PATH+'/rastermap',
                 NBSERVICES_SRC_PATH+'/singlesearch',
                 NBSERVICES_SRC_PATH+'/spatial',
                 NBSERVICES_SRC_PATH+'/tileservice',
                 NBSERVICES_SRC_PATH+'/util',
                 NBSERVICES_SRC_PATH+'/vectortile',
                 NBSERVICES_INC_PATH+'/protected',
                 NBSERVICES_SRC_PATH+'/data/data_retrieve_confirmed.c',
                 NBUI_SRC_PATH+'/android_ndk',
                 THIRDPARY_PATH+'/tinyxml',
                 THIRDPARY_PATH+'/libxml2',
     ],
    'exclude' : [
                 '*/CoreUBaseString.cpp',
                 '*/CoreUniString.cpp',
                 '*/Core*String.cpp',
                 '*/CoreAscString.cpp',
                 '*/singlesearch/SingleSearch.cpp',
    ]
}

nbnav_src = {
    'include' : [NBSERVICES_SRC_PATH+'/nav/nbgpshistory.c',
                 NBSERVICES_SRC_PATH+'/nav/nbguidancemessage.c',
                 NBSERVICES_SRC_PATH+'/nav/nav_avg_speed.c',
                 NBSERVICES_SRC_PATH+'/nav/instructset.c'
                 ],
    'exclude' : []
}

abservices_inc = [
    ABSERVICES_INC_PATH,
    ABSERVICES_INC_PATH+'/android_ndk',
    ABSERVICES_INC_PATH+'/protected'
]

abservices_private_inc = [
    ABSERVICES_INC_PATH+'/private',
]

abservices_src = {
    'include' : [ABSERVICES_SRC_PATH],
    'exclude' : []
}

mapkit_inc = [
    JNI_INC_PATH+'/map3d',
]

mapkit_src = {
    'include' : [JNI_SRC_PATH+'/map3d'],
    'exclude' : []
}

network_inc = [
    JNI_INC_PATH+'/network',
]

network_src = {
    'include' : [JNI_SRC_PATH+'/network'],
    'exclude' : []
}

ltkcommon_inc = [
    JNI_INC_PATH+'/ltkcommon',
]

ltkcommon_src = {
    'include' : [JNI_SRC_PATH+'/ltkcommon'],
    'exclude' : []
}

map_inc = [ MAP_INC_PATH ]
map_src = { 'include' : [MAP_SRC_PATH], 'exclude' : []}

common_inc = [
    COMMON_INC_PATH,
    COMMON_INC_PATH+'/android_ndk',
    COMMON_INC_PATH+'/protected'
]
common_private_inc = [ COMMON_INC_PATH + '/private' ];
common_src = { 'include' : [COMMON_SRC_PATH], 'exclude' : []}

nav_inc = [ NAV_INC_PATH ]
nav_src = { 'include' : [NAV_SRC_PATH], 'exclude' : []}

nbgm_inc = [
    NBGM_MAIN_INC_PATH,
    NBGM_MAIN_INC_PATH+'/android_ndk',
    NBGM_MAIN_INC_PATH+'/protected',
    NBRE_INC_PATH,
    NBRE_INC_PATH+'/debug',
    NBRE_INC_PATH+'/android_ndk',
    NBRE_INC_PATH+'/protected',
    GLES_INC_PATH,
    GLES_INC_PATH+'/protected',
    GLES2_INC_PATH,
    GLES2_INC_PATH+'/protected'
]

nbgm_private_inc = [
    NBGM_MAIN_INC_PATH+'/private',
    NBRE_INC_PATH+'/private',
    GLES_INC_PATH+'/private',
    GLES2_INC_PATH+'/private',
]

nbgm_src = {
    'include' : [
        NBGM_MAIN_SRC_PATH,
        NBRE_SRC_PATH,
        GLES_SRC_PATH,
        GLES2_SRC_PATH
    ],
    'exclude' : [
        '*/win32/*',
        '*/winrt/*',
        '*/iphone/*',
        '*/contrib/*',
        '*/qt_qnx/*',
        '*/qt/*'
    ]
}

nbm_src = { 'include' : [NBM_PATH+'/libs/libNBM.a'], 'exclude' : []}
nbm_inc = [ NBM_PATH+'/include' ]

ncdb_src = { 'include' : [NCDB_PATH+'/libs/libNCDB.so'], 'exclude' : []}
ncdb_inc = [ NCDB_PATH+'/include' ]

ncdbwrap_src = { 'include' : [NCDB_PATH+'/libs/libNCDBWRAP.so'], 'exclude' : []}
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
#   profiling: boolean; True if the component wants profile data and profiling is enable, default is False
#   profflag: boolean; True if the component wants PROFILE_CCC flag and profiling is enable, default is False
#   root_dir: string; root directory name of the component (normally the folder where src and include folders are located); defailt is /
#
components = {
    'MY_NBPAL' :        {'inc' : nbpal_inc, 'private' : nbpal_private_inc, 'src': nbpal_src, 'module_name' : 'nbpal', 'gen_version' : True},
    'MY_ABPAL' :        {'inc' : abpal_inc, 'private' : abpal_private_inc, 'src' : abpal_src, 'module_name' : 'abpal', 'shared_libs' : ['nbpal'], 'gen_version' : True},
    'MY_CORESERVICES' : {'inc' : coreservices_inc, 'private' : coreservices_private_inc, 'src' : coreservices_src, 'module_name' : 'coreservices', 'shared_libs' : ['nbpal'], 'gen_version': True },
    'MY_NBSERVICES' :   {'inc' : nbservices_inc, 'private' : nbservices_private_inc, 'src' : nbservices_src, 'module_name' : 'nbservices', 'shared_libs' : ['ncdb', 'nbpal', 'coreservices', 'nbgm'], 'static_libs' : ['nbdata', 'nbprotocol', 'nbm', 'common'], 'gen_version' : True, 'gen_tpslib': True, 'profiling': True, 'profflag' : True },
    'MY_NBDATA'     :   {'inc' : nbdata_inc, 'private' : nbdata_private_inc, 'src' : nbdata_src, 'module_name' : 'nbdata', 'is_static' : True, 'profiling' : False},
    'MY_NBPROTOCOL' :   {'inc' : nbprotocol_inc, 'private' : nbprotocol_private_inc, 'src' : nbprotocol_src, 'module_name' : 'nbprotocol', 'is_static' : True, 'profiling' : False},
    'MY_MAP' :          {'inc' : map_inc, 'src' : map_src, 'module_name' : 'map', 'is_static' : True, 'profiling' : False},
    'MY_COMMON' :       {'inc' : common_inc, 'private' : common_private_inc, 'src' : common_src, 'module_name' : 'common', 'is_static' : True, 'profiling' : False},
    'MY_MAPKIT' :       {'inc' : mapkit_inc, 'src' : mapkit_src, 'module_name' : 'mapkit', 'shared_libs' : ['nbservices', 'coreservices', 'nbpal', 'nbgm', 'ltkcommon'], 'static_libs' : ['map'], 'profflag': False, 'profiling' : False},
    'MY_LTKCOMMON' :    {'inc' : ltkcommon_inc, 'src' : ltkcommon_src, 'module_name' : 'ltkcommon', 'shared_libs' : ['nbservices', 'coreservices', 'nbpal', 'nbgm', 'abpal', 'abservices'], 'static_libs' : ['common'], 'profflag': False, 'profiling' : False},
    'MY_NBGM' :         {'inc' : nbgm_inc, 'private' : nbgm_private_inc, 'src' : nbgm_src, 'module_name' : 'nbgm', 'shared_libs' : ['nbpal'], 'gen_version' : True, "root_dir" : 'nbgmmain', 'rendertype' : True},
    'MY_NBM':           {'inc' : nbm_inc, 'src' : nbm_src, 'is_static' : True, 'is_prebuilt' : True, 'module_name' : 'nbm'},
    'MY_NCDB':           {'inc' : ncdb_inc, 'src' : ncdb_src, 'is_static' : False, 'is_prebuilt' : True, 'module_name' : 'ncdb'},
    'MY_ABSERVICES' :   {'inc' : abservices_inc, 'private' : abservices_private_inc, 'src' : abservices_src, 'module_name' : 'abservices', 'shared_libs' : ['coreservices', 'nbservices', 'abpal', 'nbpal'], 'static_libs' : [], 'gen_version' : True},
    'MY_NETWORK' :      {'inc' : network_inc, 'private' : nbservices_private_inc, 'src' : network_src, 'module_name' : 'network', 'shared_libs' : ['nbservices', 'coreservices', 'nbpal', 'nbgm'], 'static_libs' : [], 'profflag': False, 'profiling' : False},
    # 'MY_NAV' :          {'inc' : nav_inc,
    #                      'src' : nav_src,
    #                      'module_name' : 'nav',
    #                      'shared_libs' : ['nbservices', 'coreservices',
    #                                       'nbpal', 'ltkcommon', 'abpal'],
    #                      'profflag': False,
    #                      'profiling' : False},
    'MY_NCDBWRAP':      {'src' : ncdbwrap_src, 'is_static' : False, 'is_prebuilt' : True, 'module_name' : 'ncdbwrap'},
}

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
    def private(self):
        try:
            return self.data['private']
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
    def root_dir(self):
        try:
            return self.data['root_dir']
        except KeyError:
            return ''
    def is_prebuilt(self):
        try:
            return self.data['is_prebuilt']
        except KeyError:
            return False
    def profiling(self):
        try:
            return self.data['profiling']
        except KeyError:
            return False
    def rendertype(self):
        try:
            return self.data['rendertype']
        except KeyError:
            return False
    def profflag(self):
        try:
            return self.data['profflag']
        except KeyError:
            return False

class Junction:

    def __init__(self, dirmap, project_path):
        self.dirmap = dirmap
        self._project_path = project_path
        if not os.path.exists(self._project_path.jni()):
            os.mkdir(self._project_path.jni())

    def add(self):
        for key in self.dirmap.keys():
            self.__junction_add(key, self.dirmap[key])

    def __junction_add(self, junction, dir):
        j = self.junction_path(junction)
        f = self.real_path(dir)
        if not os.path.exists(j):
            if sys.platform == 'win32':
                subprocess.call(["junction.exe ", j , f])
            elif sys.platform == 'darwin':
                subprocess.call(["ln", "-s", f, j])

    def __jni_path(self, rel_path):
        return os.path.join(self._project_path.jni(), rel_path)

    def __ccc_path(self, rel_path):
        return os.path.join(self._project_path.p4_root(), 'client/c/core', rel_path)

    def remove(self):
        for key in self.dirmap.keys():
            self.__junction_del(key)

    def __junction_del(self, junction):
        j = self.__jni_path(junction)
        if os.path.lexists(j):
            if sys.platform == 'win32':
                subprocess.call(["junction.exe", "/D", j])
            elif sys.platform == 'darwin':
                subprocess.call(["rm", j])

class CccJunction(Junction):
    def junction_path(self, rel_path):
        return os.path.join(self._project_path.root(), 'jni', rel_path)
    def real_path(self, rel_path):
        return rel_path

class JccJunction(Junction):
    def junction_path(self, rel_path):
        return os.path.join(self._project_path.root(), rel_path)
    def real_path(self, rel_path):
        return os.path.join(self._project_path.p4_root(), 'client/java/core_java', rel_path)


ndk_profiler_mk = """
TARGET_thumb_release_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_thumb_release_CFLAGS))
TARGET_thumb_release_CFLAGS := $(filter-out -fomit-frame-pointer,$(TARGET_thumb_release_CFLAGS))
TARGET_arm_release_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_arm_release_CFLAGS))
TARGET_arm_release_CFLAGS := $(filter-out -fomit-frame-pointer,$(TARGET_arm_release_CFLAGS))
TARGET_CFLAGS := $(filter-out -ffunction-sections,$(TARGET_CFLAGS))

# include libandprof.a in the build
include $(CLEAR_VARS)
LOCAL_MODULE := andprof
LOCAL_SRC_FILES := prof/$(TARGET_ARCH_ABI)/libandprof.a
include $(PREBUILT_STATIC_LIBRARY)
"""


def openJniOutfile(filename, project_path):
    filepath = os.path.join(project_path.root(), 'jni', filename)
    if os.path.exists(filepath):
        os.chmod(filepath, stat.S_IRWXG | stat.S_IRWXO | stat.S_IRWXU)
    return open(filepath, 'w')

class MkFileGen:
    def __init__(self, project_path, config):
        self._project_path = project_path
        self._config = config
        self.file = openJniOutfile('Android.mk', self._project_path)


    def set_target_file(self, make_file):
        self.file = make_file

    def generate_header_make_file(self):
        self.file.write("BRANCH = %s\n" % NIM_CCC_BRANCH)
        self.file.write("include $(call all-subdir-makefiles)")

    def prefix(self):
        self.file.write("\nLOCAL_PATH := $(call my-dir)")
        self.file.write("\nLOCAL_SHORT_COMMANDS := true")
        if self._config.prof():
            self.file.write(ndk_profiler_mk)
        self.file.write("\ninclude $(CLEAR_VARS)")

    def prefix_seperate(self):
        self.file.write("\nLOCAL_PATH := $(call my-dir)/../")
        self.file.write("\nLOCAL_SHORT_COMMANDS := true")
        if self._config.prof():
            self.file.write(ndk_profiler_mk)

    def posfix(self, modules):
        s = """
include $(CLEAR_VARS)
LOCAL_MODULE := ccc
LOCAL_WHOLE_STATIC_LIBRARIES = staticlibs
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2  -llog -ldl
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
include $(BUILD_SHARED_LIBRARY)
"""
        #self.file.write(s.replace('staticlibs', string.join(modules, ' ')))

    def inc(self, name, dirs):
        self.file.write("\n")
        self.__gen_continue_lines(name, ['$(LOCAL_PATH)/' + path for path in dirs])

    def incs(self, list):
        self.__gen_plus_lines('MY_C_INCLUDES', list)

    def srcs(self, list):
        self.__gen_plus_lines('LOCAL_SRC_FILES', list)

    def src(self, dirmap):
        files = []
        for file in self.__all_files(dirmap['include'], dirmap['exclude']):
            fname, fExtension = os.path.splitext(file)
            ext = fExtension.lower()
            if ext in ('.c', '.cpp', '.cxx', '.a', '.so'):
                if sys.platform == 'win32':
                    files.append(file.replace(self._project_path.jni() + "\\", '').replace('\\', '/'))
                elif sys.platform == 'darwin':
                    files.append(file.replace(self._project_path.jni() + "/", ''))
                #files.append(file.replace(PROJ_JNI, '$(LOCAL_PATH)').replace('\\', '/'))
                #files.append(file.replace('\\', '/'))
        self.__gen_continue_lines('LOCAL_SRC_FILES', files)

    def src_pre(self, component):
        s = """
include $(CLEAR_VARS)
LOCAL_MODULE := component
"""
        self.file.write(s.replace('component', component.module_name()))
        if not component.is_prebuilt():
            s2 ="""LOCAL_CFLAGS := %CFLAGS%
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lz -lEGL -landroid -ldl
"""
            s2 = s2.replace("%CFLAGS%", self._config.cflags(component))
            self.file.write(s2)
            if len(component.private()) != 0:
                self.__gen_continue_lines('LOCAL_C_INCLUDES', ['$(LOCAL_PATH)/' + path for path in component.private()])
                self.file.write(' \\\n')
                #self.__gen_plus_lines('LOCAL_C_INCLUDES', ['MY_C_INCLUDES'])
                self.file.write('\t$(MY_C_INCLUDES)\n')
            else:
                self.file.write('LOCAL_C_INCLUDES := $(MY_C_INCLUDES)\n')

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
            if i:
                self.file.write(' \\\n\t' + s)
            else:
                self.file.write(name + ' := ' + s)


    def __gen_plus_lines(self, name, list):
        self.file.write('\n')
        for i in range(len(list)):
            if i:
                op = ' += '
            else:
                op = ' := '
            self.file.write(name + op + '$(' + list[i] + ')\n')

    def __gen_include_static_lib(self, module):
        self.file.write('\n')
        self.file.write('include $(BUILD_STATIC_LIBRARY)\n')

    def __gen_local_module(self, module):
        self.file.write('\n')
        self.file.write('LOCAL_MODULE := ' + module)

    def __all_files(self, roots, exclude_patterns='*', single_level=False, yield_folders=False):
        # Expand patterns from semicolon-separated string to list
        for root_name in roots:
            root = os.path.join(self._project_path.jni(), root_name)
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
                    for pattern in exclude_patterns:
                        if fnmatch.fnmatch(full_name, pattern):
                            include = False
                            break
                    if include:
                        yield full_name
                if single_level:
                    break

class MkFile:
    def __init__(self, project_path, config):
        self.modules = []
        self.components = []
        self._project_path = project_path
        self._config = config
        self._files = {}


    def open_component_jni_files(self):
        for c in self.components:
            folder = os.path.join(self._project_path.root(), 'jni', c.module_name())
            if not os.path.exists(folder):
                os.mkdir(folder)
            filepath = os.path.join(folder, 'Android.mk')
            if os.path.exists(filepath):
                os.chmod(filepath, stat.S_IRWXG | stat.S_IRWXO | stat.S_IRWXU)
            cfile = open(filepath,'w')
            self._files[c.module_name()] = cfile

    def close_component_jni_files(self):
        if self._files:
            for k,v in self._files.iteritems():
                v.close()


    def add_component(self, component):
        self.components.append(component)

    def gen_version(self):
        for c in self.components:
            if c.gen_version():
                subprocess.call(['python', self._project_path.py_version(c.dir_name(), c.root_dir()), self._project_path.dir(c.dir_name(), c.root_dir()), 'android_ndk'])

    def gen_tpslib(self):
        for c in self.components:
            if c.gen_tpslib():
                subprocess.call(['python', self._project_path.py_util_build_android(), self._project_path.build()], cwd = self._project_path.build())


    def gen(self):
        mkgen = MkFileGen(self._project_path, self._config)
        mkgen.prefix()
        for c in self.components:
            mkgen.inc(c.name(), c.inc())
        mkgen.incs([c.name() for c in self.components])
        for c in self.components:
            mkgen.src_pre(c)
            mkgen.src(c.src())
            mkgen.src_post(c.is_static(), c.is_prebuilt(), c.shared_libs(), c.static_libs())
        #mkgen.srcs(self.srcmap.keys())
        mkgen.posfix(self.modules)

    def gen_seperate_make_files(self):
        self.open_component_jni_files()
        mkgen = MkFileGen(self._project_path, self._config)
        mkgen.generate_header_make_file()
        for c in self.components:
            mkgen.set_target_file(self._files[c.module_name()])
            mkgen.prefix_seperate()
            for i in self.components:
                mkgen.inc(i.name(), i.inc())
            mkgen.incs([i.name() for i in self.components])
            mkgen.src_pre(c)
            mkgen.src(c.src())
            mkgen.src_post(c.is_static(), c.is_prebuilt(), c.shared_libs(), c.static_libs())
            mkgen.posfix(self.modules)
        self.close_component_jni_files()


class ClasspathFileGen:
    def __init__(self, project_path):
        self._project_path = project_path
        filename = os.path.join(self._project_path.root(), '.classpath')
        if os.path.exists(filename):
            os.chmod(filename, stat.S_IRWXU)
        self.file = open(filename, 'w')

    def gen(self):
        classpath ="""<?xml version="1.0" encoding="UTF-8"?>
<classpath>
    <classpathentry kind="src" path="src_nbui"/>
    <classpathentry kind="src" path="src"/>
    <classpathentry kind="src" path="gen"/>
    <classpathentry kind="con" path="com.android.ide.eclipse.adt.ANDROID_FRAMEWORK"/>
    <classpathentry kind="con" path="com.android.ide.eclipse.adt.LIBRARIES"/>
    <classpathentry kind="output" path="bin/classes"/>
</classpath>
"""
        self.file.write(classpath)

GNUSTL = """
APP_PLATFORM := android-9
APP_STL := gnustl_shared
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -DANDROID
"""

STLPORT = """
APP_PLATFORM := android-9
APP_STL := stlport_shared
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CPPFLAGS += -fpermissive
APP_CPPFLAGS += -DANDROID
LOCAL_SHORT_COMMANDS := true
HOST_ECHO := echo
APP_SHORT_COMMANDS := true
"""

class ApplicationFileGen:
    def __init__(self, project_path):
        self._file = openJniOutfile("Application.mk", project_path)

    def gen(self, stlport):
        abi = "APP_ABI := " + CPU_TYPE
        if stlport:
            self._file.write(STLPORT + abi)
        else:
            self._file.write(GNUSTL + abi)


options = [
    optparse.make_option("--prof", action="store_true", dest="prof", default=False, help="Enable profiling of nbservices.so"),
    optparse.make_option("--stlport", action="store_true", dest="stlport", default=False, help="Linked with stlport instead of gnustl"),
    optparse.make_option("--jpal", action="store_true", dest="jpal", default=False, help="Uses java implementation of nbpal.")
]

DEFAULT_FLAGS = [
    "ANDROID_NDK",
    "DEBUG_LOG_DEBUG",
    "GL_GLEXT_PROTOTYPES=1",
    "FT2_BUILD_LIBRARY",
    "FT_OPTION_AUTOFIT2",
    "FLOATING_POINT",
    "NDEBUG" # @note: we should only define NDEBUG for release build, this is
             #        implemented in cmake scripts.
]

STLPORT_FLAGS = [
    "USE_STLPORT",               # use stlport instead of gnustl
    "BOOST_EXCEPTION_DISABLE",   # only needed if USE_STLPORT is defined
    "BOOST_NO_EXCEPTION",        # only needed if USE_STLPORT is defined
]

JPAL_FLAGS = [
    "JPAL",
]

RENDERTYEP_FLAGS = [
    "SUPPORT_GLES",
    "SUPPORT_GLES20",
]

class Config:
    def __init__(self, args):
        parser = optparse.OptionParser(option_list=options)
        self._options, self._args = parser.parse_args()

    def prof(self):
        return self._options.prof

    def stlport(self):
        return self._options.stlport

    def jpal(self):
        return self._options.jpal

    # need this flags since profiling is per component base
    def cflags(self, component):
        flags = string.join(["-D" + e for e in DEFAULT_FLAGS])
        if component.profiling() and self.prof():
            flags += " -pg"
        if component.profflag() and self.prof():
            flags += " -DPROFILE_CCC"
        if self.stlport():
            flags += string.join([ " -D" + e for e in STLPORT_FLAGS])
        if self.jpal():
            flags += string.join([ " -D" + e for e in JPAL_FLAGS])
        if component.rendertype():
            flags += string.join([ " -D" + e for e in RENDERTYEP_FLAGS])
        return flags


class Generator:
    def __init__(self, project_path, config):
        self._components = components
        self._project_path = project_path
        self._config = config

    def gen(self):
        self.__junction()
        mkfile = MkFile(self._project_path, self._config)
        for key in sorted(self._components.keys()):
            value = self._components[key]
            component = Component(key, value)
            mkfile.add_component(component)
        mkfile.gen_version()
        mkfile.gen_tpslib()

        if SEPERATE_MAKE_FLAG:
            mkfile.gen_seperate_make_files()
        else:
            mkfile.gen()
        classpathFileGen = ClasspathFileGen(self._project_path)
        classpathFileGen.gen()
        applicationFileGen = ApplicationFileGen(self._project_path);
        applicationFileGen.gen(self._config.stlport())

    def __junction(self):
        cccJunction = CccJunction(self.__get_ccc_dirmap(), self._project_path)
        cccJunction.remove()
        cccJunction.add()
        jccJunction = JccJunction(self.__get_jcc_dirmap(), self._project_path)
        jccJunction.remove()
        jccJunction.add()


    def __get_ccc_dirmap(self):
        for key in ccc_dirmap:
            if key == NBM_PATH:
                ccc_dirmap[key] = self._project_path.nbm()
                continue
            elif key == NCDB_PATH:
                ccc_dirmap[key] = self._project_path.ncdb()
                continue
            elif key == 'prof':
                ccc_dirmap[key] = self._project_path.prof()
                continue
            elif key in ['absrvcont_inc', 'absrvcont_src', 'scui_inc', 'scui_src']:
                ccc_dirmap[key] = self._project_path.appcommon(ccc_dirmap[key])
                continue
            value = ccc_dirmap[key]
            ccc_dirmap[key] = self._project_path.ccc(value)
        return ccc_dirmap

    def __get_jcc_dirmap(self):
        return jcc_dirmap


def main(args=None):
    P4_ROOT = os.path.abspath('../../../../../../')
    PROJ_ROOT = os.path.abspath('./')
    BRANCH = NIM_CCC_BRANCH
    BRANCH_APP_COMMON = os.environ['NIM_APP_COMMON_BRANCH']

    BUILD_ROOT = os.path.join(P4_ROOT, 'client/c/core/build', BRANCH)

    my_project_path = ProjectPath(P4_ROOT, PROJ_ROOT, BRANCH, BRANCH_APP_COMMON)
    my_project_path.print_dirs()

    config = Config(args)
    if config.stlport(): # if we are using stlport, we need to locate the correct path
        global nbm_src
        nbm_src['include'] = [NBM_PATH + "/libs/portstl/libNBM.a"]

    generator = Generator(my_project_path, config)
    generator.gen()


if __name__=='__main__':
    main()

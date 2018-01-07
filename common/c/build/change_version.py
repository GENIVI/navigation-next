import sys
import os
import re
from tempfile import mkstemp
from shutil import move

def_files = [
		"abpal/%s/projects/winmobile/abpal.def",
		"abservices/%s/projects/winmobile/abservices.def",
		"coreservices/%s/projects/winmobile/coreservices.def",
		"nbpal/%s/projects/winmobile/nbpal.def",
		"nbservices/%s/projects/winmobile/nbservices.def"
		]

proj_files = [
		"abpal/%s/projects/win32/abpal.vcproj",
		"abpal/%s/projects/win32/unittests.vcproj",
		"abpal/%s/projects/winmobile/abpal.vcproj",
		"abpal/%s/projects/winmobile/unittests.vcproj",
		"abservices/%s/projects/win32/abservices.vcproj",
		"abservices/%s/projects/winmobile/abservices.vcproj",
		"coreservices/%s/projects/win32/coreservices.vcproj",
		"coreservices/%s/projects/win32/unittests.vcproj",
		"coreservices/%s/projects/winmobile/coreservices.vcproj",
		"coreservices/%s/projects/winmobile/unittests.vcproj",
		"nbpal/%s/projects/win32/nbpal.vcproj",
		"nbpal/%s/projects/win32/unittests.vcproj",
		"nbpal/%s/projects/winmobile/nbpal.vcproj",
		"nbpal/%s/projects/winmobile/unittests.vcproj",
		"nbservices/%s/projects/win32/nbservices.vcproj",
		"nbservices/%s/projects/winmobile/nbservices.vcproj",
		"systemtests/%s/projects/win32/System Tests.vcproj",
		"systemtests/%s/projects/winmobile/System Tests.vcproj"
		]

version_files = [
		"abpal/%s/resources/android_ndk/version.txt",
		"abpal/%s/resources/brew/version.txt",
		"abpal/%s/resources/iphone/version.txt",
		"abpal/%s/resources/linux/version.txt",
		"abpal/%s/resources/qt_qnx/version.txt",
		"abpal/%s/resources/symbian/version.txt",
		"abpal/%s/resources/win32/version.txt",
		"abpal/%s/resources/winmobile/version.txt",
		"abservices/%s/resources/version.txt",
		"coreservices/%s/resources/version.txt",
		"nbpal/%s/resources/android_ndk/version.txt",
		"nbpal/%s/resources/brew/version.txt",
		"nbpal/%s/resources/iphone/version.txt",
		"nbpal/%s/resources/linux/version.txt",
		"nbpal/%s/resources/qt_qnx/version.txt",
		"nbpal/%s/resources/symbian/version.txt",
		"nbpal/%s/resources/win32/version.txt",
		"nbpal/%s/resources/winmobile/version.txt",
		"nbservices/%s/resources/version.txt",
		"build/%s/version_android_ndk.txt",
		"build/%s/version_brew.txt",
		"build/%s/version_iphone.txt",
		"build/%s/version_qt_qnx.txt",
		"build/%s/version_webos.txt",
		"build/%s/version_win32.txt",
		"build/%s/version_winmobile.txt"
		]

def update_path(files, ccc_root, branch):
	for i in range(len(files)):
		files[i] = os.path.normpath( os.path.join(ccc_root, files[i] % branch) )
	
def multiple_replace(text, dict):
	# Create a regular expression  from the dictionary keys
	regex = re.compile("(%s)" % "|".join(map(re.escape, dict.keys())))

	# For each match, look-up corresponding value in dictionary
	return regex.sub(lambda mo: dict[mo.string[mo.start():mo.end()]], text) 

def file_search_replace(file, dict):
	temp_fh, temp_abspath = mkstemp()
	new_file = os.fdopen(temp_fh, "w")
	old_file = open(file)
	for line in old_file:
		new_file.write(multiple_replace(line, dict))

	new_file.close()

	old_file.close()
	move(temp_abspath, file)

def update_version_file(file, old_version, new_version):
	fh = open(file, "w")
	fh.write(new_version)
	fh.close()

def get_versions(version):
	(major, minor, subversion, build) = ("0", "0", "0", "0")

	versions = version.split(".")

	if len(versions) > 0:
		major = versions[0]

	if len(versions) > 1:
		minor = versions[1]

	if len(versions) > 2:
		subversion = versions[2]

	if len(versions) > 3:
		build = versions[3]

	return (major, minor, subversion, build)

if __name__ == "__main__":
	if (len(sys.argv) < 3):
		print "Usage python change_version.py <old_version> <new_version>"
		print "Example: python change_version.py 1.0.0.0 1.5.0.0"
		sys.exit(1)

	#find path
	script_path =os.path.abspath(os.path.dirname(sys.argv[0]))
	branch = os.path.basename(script_path)
	ccc_root = os.path.abspath(os.path.join(script_path, "..", ".."))

	#update full path
	update_path(def_files, ccc_root, branch);
	update_path(proj_files, ccc_root, branch);
	update_path(version_files, ccc_root, branch);

	old_version = sys.argv[1]
	new_version = sys.argv[2]

	(old_major, old_minor, old_subversion, old_build) = get_versions(sys.argv[1])
	(new_major, new_minor, new_subversion, new_build) = get_versions(sys.argv[2])

	old_version = "%s.%s.%s.%s" % (old_major, old_minor, old_subversion, old_build)
	new_version = "%s.%s.%s.%s" % (new_major, new_minor, new_subversion, new_build)

	replace_dict = {}

	for comp in ("abpal", "nbpal", "coreservices", "nbservices", "abservices"):
		# abpal10 - > abpal->15
		old_str = "%s%s%s" % (comp, old_major, old_minor)
		new_str = "%s%s%s" % (comp, new_major, new_minor) 

		# "abpal10" -> "abpal15"
		replace_dict[ "\"%s\"" % old_str ] = "\"%s\"" % new_str

		# abpal10.dll -> abpal15.dll
		replace_dict[ "%s.dll" % old_str] = "%s.dll" % new_str

		# abpal10.lib -> abpal15.lib
		replace_dict[ "%s.lib" % old_str] = "%s.lib" % new_str

		# abpal10.pdb -> abpal15.pdb
		replace_dict[ "%s.pdb" % old_str] = "%s.pdb" % new_str

	for f in def_files:
		print "update def file", f
		os.system("p4 edit \"%s\"" % f)
		file_search_replace(f, replace_dict)

	for f in proj_files:
		print "update project file", f
		os.system("p4 edit \"%s\"" % f)
		file_search_replace(f, replace_dict)

	for f in version_files:
		print "update version file", f
		os.system("p4 edit \"%s\"" % f)
		update_version_file(f, old_version, new_version)


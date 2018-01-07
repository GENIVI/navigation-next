import os
import sys

def usage():
  print
  print 'Usage: python tpslib_to_char_array.py tpslib qalogtpslib package outfile'

def parse_args():
  args = sys.argv[1:]
  if len(args) == 2:
    return args[0], args[1], "tpslib"
  elif len(args) == 3:
    return args[0], args[1], args[2]
  else:
    usage()
    sys.exit()

def write_package(fout, package):
  fout.write("package %s;\n\n" % package)

def write_import(fout):
  fout.write("\n")

def write_classname_begin(fout, outfile):
  fout.write("static unsigned char tpsdata = {\n")

def get_func_name(i, prefix):
  line = "%prefix%_content"
  line = line.replace("%prefix%", prefix)
  line1 = "%d" % i
  line = line + line1
  return line

def get_max_name(prefix):
  line = "%prefix%_MAX_LEN"
  line = line.replace("%prefix%", prefix)
  return line

def write_content_max_length(fout, len, prefix):
  line = "private static final int %MAX_LEN% = %len%;\n\n"
  line1 = "%d" % len
  line = line.replace("%len%", line1)
  line1 = get_max_name(prefix)
  line = line.replace("%MAX_LEN%", line1)
  fout.write(line)

def write_content_begin(fout, arrname):
  fout.write("static const unsigned char %s[] = \n{" % arrname)

def write_content_data(fout, s):
  for i in range(len(s)):
    if (i) % 8 == 0:
      fout.write("\n\t")
    c = s[i]
    fout.write("0x%02x, " % ord(c))

def write_content_end(fout):
  fout.write("\n};\n");

# returns an array of strings each of which has max len of param size
def partition_conent(tpslib, size):
  s = open(tpslib, 'rb').read()
  t = 0
  a = []
  slen = len(s)
  while t < slen:
    a.append(s[t:t+size])
    t = t + size
  return a, slen

def write_content_get_data_begin(fout):
  fout.write("\n\n")
  fout.write("public static final InputStream getInputStream(int networkType) {\n")
  fout.write("\tStringBuffer sb = null;\n");
  fout.write("\tswitch(networkType){\n");

def write_content_get_data(fout, num, select, prefix):
  line = "\t\tcase %select%:\n"
  line = line.replace("%select%", select)
  fout.write(line)
  line = "\t\t\tsb = new StringBuffer(%MAX_LEN%);\n"
  line = line.replace("%MAX_LEN%", get_max_name(prefix))
  fout.write(line);
  for i in range(num):
    fout.write("\t\t\tsb.append(%s());\n" % get_func_name(i, prefix))
  fout.write("\t\t\treturn new StringBufferInputStream(sb.toString());\n")

def write_content_get_data_end(fout):
  fout.write("\t}\n\treturn null;\n}\n")


def write_content(fout, tpslib, prefix):
  a, tlen = partition_conent(tpslib, 1024)
  write_content_max_length(fout, tlen, prefix)
  for i in range(len(a)):
    write_content_begin(fout, i, prefix)
    write_content_data(fout, a[i])
    write_content_end(fout)
  return len(a)


def write_classname_end(fout):
  fout.write("};\n")

def output_file(filename):
#  print open(filename).readlines()
  print

def create_outfile(outfile):
  dir = os.path.dirname(outfile)
  if dir != '' and not os.path.exists(dir):
    os.mkdir(dir)
  return open(outfile, 'w')

def main(argv=None):
    tpslib, outfile, arrname = parse_args()

    fout = create_outfile(outfile)

    s = open(tpslib, 'rb').read()

    write_content_begin(fout, arrname)
    write_content_data(fout, s)
    write_content_end(fout)

    fout.close()
    output_file(outfile)

if __name__ == '__main__':
    main()

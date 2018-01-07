import os
import shutil
import sys

"""
updateversion.py
This program update a TPS template version.
"""

def create_outfile(outfile):

  dir = os.path.dirname(outfile)

  if dir != '' and not os.path.exists(dir):

    os.mkdir(dir)

  return open(outfile, 'w')

  

def getVersion(filepath):
    assert filepath != None
    fhandle = open(filepath, 'rU')
    return fhandle.readlines()[0];

def updateVersion(version):
    fin = open("./tpslib.txt", 'r');
    fout = create_outfile("./tpslib_tmp.txt");
 
    for eachline in fin:
        if eachline == '  version:\n':
            print eachline,;  
            fout.write('  version: %s\n' % version);
        else:
            fout.write(eachline);

    
def main(argv=None):
    version = getVersion("./version.txt");
    updateVersion(version);
    
    return 0

if __name__ == '__main__':
    main()

import subprocess
import sys
import datetime
import time
import os
import shutil

ResultPath = '.\TestsResult\Result.txt'
SysTestsPath = '.\systemtests.exe'
mainc = '..\..\..\src\main.c'

if (os.path.exists('TestsResult') == True):
	shutil.rmtree('TestsResult')
	os.makedirs('TestsResult')
else:
	os.makedirs('TestsResult')

main = open(mainc, 'r')
line = []
flag = 0
NotCommented = 0
checkComment = []
testfunc = 0

for line in main:
	flag2 = 0
	testfunc = 0
	slash = line.find('//')
	# '/*' won't be an open comment if you use it after '//' key
	# that's why we should replace '/*' keys which are located
	# after'//' key to empty char
	if (slash > -1):
		line = line[:slash] + '//' + line[slash+2:].replace('/*',' ')
	# check that current line contain 'CreateTestSuite' function
	# and there are no any '//' key before 'CreateTestSuite'
	last = ' ' #previous character in the line
	last2 = ' ' #previous 2 characters in the line
	i = 0

	for char in line:
		if (last2 == '/*' and char == '/'):
			# if parser will find '/' key after '/*'
			# it should not be construed as '*/' key
			continue
		if (last2 == '*/' and char == '*'):
			continue

		if (last == '/' and char == '/'): # if '//'
			flag2 = 1
		if (last == '/' and char == '*'): # if '/*'
			flag = 1
		if (last == '*' and char == '/'): # if '*/'
			flag = 0
			flag2 = 0
		last2 = last+char
		last = char
		i = i + 1
		if (i == line.find('CreateTestSuite') and flag == 0 and flag2 == 0):
			NotCommented = NotCommented + 1

main.close()
# Beside the required 'CreateTestSuite' calling, in main.c
# should be declaration and definition of the function.
# We need to remove them
NotCommented = NotCommented - 2
NUM_Suite = NotCommented
if (NUM_Suite == 0):
	sys.exit()

StartTime = int (time.mktime(datetime.datetime.now().timetuple())) #Check the time(in sec)
class typeinfo: # For Result.txt parser
	total = 0
	ran = 0
	passed = 0
	failed = 0

class testtime:	# For Result.txt parser
	min = 0
	sec = 0

if (len(sys.argv) > 1):
	NUM_Proc = int(sys.argv[1])			#Number of processes
	if (NUM_Proc > NUM_Suite):
		NUM_Proc = NUM_Suite
	if (NUM_Proc <= 0):
		NUM_Proc = 1
else:
	NUM_Proc = 1

step = NUM_Suite / NUM_Proc	# Number of suites per proccess
iter = 1
CNT_Proc = 0
left = NUM_Suite			# Number of remaining suites
handleproc = []

while left > 0:
	time.sleep(0.5)
	CNT_Proc = CNT_Proc + 1
	if CNT_Proc == NUM_Proc:
		step = left
	handleproc.append(subprocess.Popen(SysTestsPath + ' -P ' + str(iter) + ' -S ' + str(step), creationflags = subprocess.CREATE_NEW_CONSOLE ))
	left = left - step
	iter = iter + step

for proc in handleproc:
	proc.wait()				# Wait for all processes

# Handling of the results
FileRes = open(ResultPath, 'r') # File Result.txt is created by systemtests.exe
suites = typeinfo()
tests = typeinfo()
asserts = typeinfo()
line = []

for line in FileRes:
	wordsLine = line.split()
	if len(wordsLine) == 0:
		continue
	if wordsLine[0] == 'suites':
		suites.total = suites.total + int(wordsLine[1])
		suites.ran = suites.ran + int(wordsLine[2])
		suites.failed = suites.failed + int(wordsLine[4])
	if wordsLine[0] == 'tests':
		tests.total = tests.total + int(wordsLine[1])
		tests.ran = tests.ran + int(wordsLine[2])
		tests.passed = tests.passed + int(wordsLine[3])
		tests.failed = tests.failed + int(wordsLine[4])
	if wordsLine[0] == 'asserts':
		asserts.total = asserts.total + int(wordsLine[1])
		asserts.ran = asserts.ran + int(wordsLine[2])
		asserts.passed = asserts.passed + int(wordsLine[3])
		asserts.failed = asserts.failed + int(wordsLine[4])
FileRes.close()

EndTime = int (time.mktime(datetime.datetime.now().timetuple())) # Check the finish time
time = testtime()
time.min = (EndTime - StartTime) / 60
time.sec = (EndTime - StartTime) % 60

# Write parsed results to the Result.txt file
ParsedResults = open(ResultPath, 'w')
ParsedResults.write('\n--Run Summary: Type      Total   Ran    Passed  Failed')
ParsedResults.write('\n               suites'+'\t '+str(suites.total) + '\t ' + str(suites.ran) + '\t' + 'n/a' + '\t ' + str(suites.failed))
ParsedResults.write('\n               tests'+'\t '+str(tests.total) + '\t ' + str(tests.ran) + '\t' + str(tests.passed) + '\t ' + str(tests.failed))
ParsedResults.write('\n               asserts'+'\t '+str(asserts.total) + '\t ' + str(asserts.ran) + '\t' + str(asserts.passed) + '\t ' + str(asserts.failed))
ParsedResults.write('\n\nTest Time: ' + str(time.min) + ' minutes, ' + str(time.sec) + ' seconds')
ParsedResults.close()
import os
import sys
import time
import math
import csv
import shutil
from btcomparator_for_route import compareRoute

"""
Batch Tool Comparator for navigation session
"""

count = [0,0,0] # number of all call callback, number of missed off-callback, number of missed on-callback

def compareStr(strA, strB):
    res = 0
    if strA == strB:
        res = 100
    elif strA.lower() == strB.lower():
        res = 80
    else:
        if len(strA) and len(strB):
            strALow=strA.lower()
            strBLow=strB.lower()
            wordsA=strALow.split()
            wordsB=strBLow.split()

            if len(wordsA) and len(wordsB):
                matchedCount=0
                if len(wordsA) > len(wordsB):
                    for wordB in wordsB:
                        if strALow.find(wordB)!=-1:
                            matchedCount=matchedCount+1
                    res = float(matchedCount) / float(len(wordsA)) * 100

                else:
                    for wordA in wordsA:
                        if strBLow.find(wordA)!=-1:
                            matchedCount=matchedCount+1
                    res = float(matchedCount) / float(len(wordsB)) * 100

    return res


def printResult( pr, firstStr, secondStr,flag, wr):

    if len ( firstStr )== 0 :
        firstStr = [secondStr[0], '', '']

    if len ( secondStr ) == 0 :
        secondStr = [firstStr[0], '', '']

    if flag == 0:
        offStr = firstStr
        onStr = secondStr
    else:
        onStr = firstStr
        offStr = secondStr

    count[0] +=1

    try:
        if pr == 0:
            wr.writerow([offStr[0], 'missed', offStr[1], onStr[1], offStr[2], onStr[2]])
            if (flag == 0):
                count[1] += 1
            else:
                count[2] += 1
        else:
            wr.writerow([offStr[0], pr, offStr[1], onStr[1], offStr[2], onStr[2]])
    except Exception, e:
        print 'Error in input parameters for printResult\n'

def main(argv=None):
    """
    MAIN METHOD
    """
    print ""
    print "*************************************************"
    print "*** BatchTool NavigationSession Comparator    ***"
    print "*************************************************"
    print ""

    if argv is None:
        argv = sys.argv
    progname = argv[0]
    if progname is None:
        progname = "btcomparator_for_route.py"

    if len(argv) < 3:
        print "Parameters incorrect, comparator has to run with following parameters: input_off.csv, input_on.csv, output_res.csv"
        return 2

    OffboardInputFile = argv[1]
    OnboardInputFile = argv[2]
    resultFile = argv[3]

    print "Comparision %s vs %s " % (OffboardInputFile, OnboardInputFile)

    fOff = open(OffboardInputFile, 'r')
    fOn = open(OnboardInputFile, 'r')
    fResult = open(resultFile, 'w')

    firstCsv = csv.reader(fOff)
    secondCsv = csv.reader(fOn)
    csvWriter = csv.writer(fResult,  lineterminator = '\n' )

    csvWriter.writerow(['#CaseId', 'Matched', 'Callback off', 'Callback on', 'Info off', 'Info on'])

    iter1 = 2
    iter2 = 2
    fExit = False
    while not fExit:
        fTemp = open('route.txt', 'w')

        flag = False
        listNavigation = []
        routeInformation = []

        listNavigationSecond = []
        routeInformationSecond = []
        i = 0
        for row in firstCsv:

            if ( row[0] == '#CaseId'): continue
            if (iter1 <= int(row[0])):
                iter1 += 1
                break

            if(flag == True):
                try:
                    k = int(row[1])
                    routeInformation.append(row)
                    i += 1
                    if( i == k):
                        flag = False
                        i = 0
                    continue
                except Exception, e:
                    pass

            if (row[1] == "Route Information"):
                flag = True
                continue

            listNavigation.append(row)

        flag = False
        i = 0
        for row in secondCsv:

            if ( row[0] == '#CaseId'): continue
            if (iter2 <= int(row[0])):
                iter2 += 1
                break

            if(flag == True):
                try:
                    k = int(row[1])
                    routeInformationSecond.append(row)
                    i += 1
                    if( i == k):
                        flag = False
                        i = 0
                    continue
                except Exception, e:
                    pass

            if (row[1] == "Route Information"):
                flag = True
                continue

            listNavigationSecond.append(row)

        else:
            fExit = True

        lenListFirst = len(listNavigation)
        lenListSecond = len(listNavigationSecond)

        listFirst = listNavigation
        listSecond = listNavigationSecond
        flag = 0 # 0 - listFirst, 1 - listSecond

        ln = max(lenListFirst, lenListSecond) + 1
        print '2\n'
        while 1:
            tmp = []
            pr = 0

            if (len(listFirst) == 0):
                for rw in listSecond:
                    printResult( 0, rw, '', flag, csvWriter)
                break

            if (len(listSecond) == 0):
                for rw in listFirst:
                    printResult( 0, rw, '', flag, csvWriter)
                break

            if (listFirst[0][1] == listSecond[0][1]):
                pr = compareStr(listFirst[0][2], listSecond[0][2])
                if ( pr == 0): pr += 30
                tmp = listSecond[0]
            else:
                for rw in listSecond:
                    if rw[1] == listFirst[0][1]:
                        tmp = rw
                        try:
                            pr = compareStr(listFirst[0][2], tmp[2])
                            if ( pr == 0): pr += 30
                        except Exception, e:
                            pass
                        break

            printResult( pr, listFirst[0], tmp, flag, csvWriter)

            listFirst.remove(listFirst[0])
            if (tmp != []):
                listSecond.remove(tmp)
            if (pr == 0 ):
                if flag == 0:
                    listFirst = listNavigationSecond
                    listSecond = listNavigation
                    flag = 1
                else:
                    listFirst = listNavigation
                    listSecond = listNavigationSecond
                    flag = 0

        if  len(routeInformation) != 0 and len(routeInformationSecond) != 0:

            try:
                prStrOff = 'Callback off: %f'%(float(count[0] - count[2])/float(count[0])*100)
                prStrOn = 'Callback off: %f'%(float(count[0] - count[1])/float(count[0])*100)
                prStrMissedOff = 'missed Off: %f'%(float(count[1])/float(count[0] - count[2])*100)
                prStrMissedOn = 'missed Off: %f'%(float(count[2])/float(count[0] - count[1])*100)
                prStrMatched = 'Matched %f'%(float(count[0]- count[1]-count[2])/float(count[0])*100)

                count[0] = 0
                count[1] = 0
                count[2] = 0
                str = '%s; %s; %s; %s; %s;'%(prStrOff, prStrOn, prStrMissedOff, prStrMissedOn, prStrMatched)

                if (len(routeInformation) > len(routeInformationSecond)):
                    routesFirst = routeInformation
                    routesSecond = routeInformationSecond
                else:
                    routesFirst = routeInformationSecond
                    routesSecond = routeInformation

                compareRoute(routeInformation[:int(routeInformation[0][1])], routeInformationSecond[:int(routeInformationSecond[0][1])], fTemp)
                fTemp.close()
                fTemp = open('route.txt', 'r')
                fileStr = fTemp.readlines()

                csvWriter.writerow(['', 'Information about routes', routeInformation[:int(routeInformation[0][1])], routeInformationSecond[:int(routeInformationSecond[0][1])], fileStr])
                csvWriter.writerow(['', 'Summary', str])
                fTemp.close()
            except Exception, e:
                    pass

    fOff.close()
    fOn.close()
    fResult.close()

if __name__ == '__main__':
    main()

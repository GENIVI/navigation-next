import os
import sys
import time
import math
import csv
import shutil

"""
Batch Tool Comparator fo route
"""

COL_INDEX = {
    "ID": 0,
    "CountManeuver": 1,
    "IndexManeuver": 2,
    "ImageCode": 3,
    "TurnInformation": 4,
    "Point": 5,
    "Origin": 6,
    "Destination": 7,
    "Mode":8
    }

def routeWriteToFile(route, f):
    try:
        f.write('Count of route:\n')
        f.write(route[0][COL_INDEX['CountManeuver']])
        f.write('\nOrigion of route:\n')
        f.write(route[0][COL_INDEX['Origin']])
        f.write('\nDestination of route:\n')
        f.write(route[0][COL_INDEX['Destination']])
        f.write('\nMode of route:\n')
        f.write(route[0][COL_INDEX['Mode']])
        f.write('\nManeuvers: index, image code, turn information, point\n')
        for raw in route:
            f.write('||'.join(raw[2:6]))
            f.write('\n\n')
    except Exception, e:
        f.write("Exception\n")

def compareInformation(first, second):

    pr = 100
    if(len(first) > len(second)):
        firstStr = first
        secondStr = second
    else:
        firstStr = second
        secondStr = first

    list = firstStr.split(' ')

    for rw in list:
        if(secondStr.find(rw) == -1):
            pr -= pr/len(list)

    return pr

def compareManeuver(firstManeuver, secondManeuver, f):

    result = True

    if(firstManeuver[COL_INDEX["ImageCode"]] == secondManeuver[COL_INDEX["ImageCode"]]):
        if(firstManeuver[COL_INDEX["TurnInformation"]] == secondManeuver[COL_INDEX["TurnInformation"]]):
            pass
        else:
            pr = compareInformation(firstManeuver[COL_INDEX["TurnInformation"]],secondManeuver[COL_INDEX["TurnInformation"]])

            if (pr < 80 ):
                str =  'TurnInformation are not same for maneuver with index %s\n'%firstManeuver[COL_INDEX["IndexManeuver"]]
                f.write(str)

        if(firstManeuver[COL_INDEX["Point"]] == secondManeuver[COL_INDEX["Point"]]):
            pass
        else:
            firstPoint = firstManeuver[COL_INDEX["Point"]].split(',')
            secondPoint = secondManeuver[COL_INDEX["Point"]].split(',')

            firstPointX = float(firstPoint[0])
            firstPointY = float(firstPoint[1])
            secondPointX = float(secondPoint[0])
            secondPointY = float(secondPoint[1])

            if ((firstPointX - 0.01 < secondPointX < firstPointX + 0.01) and
                (firstPointY - 0.01 < secondPointY < firstPointY + 0.01)):
                pass
            else:
                str =  'Point are not same for maneuver with index %s\n'%firstManeuver[COL_INDEX["IndexManeuver"]]
                f.write(str)

    else:
        str = 'Maneuver with index %s and imageCode %s absent\n'%(firstManeuver[COL_INDEX["IndexManeuver"]],firstManeuver[COL_INDEX["ImageCode"]])
        f.write(str)
        result = False

    return result

def compareRoute(first,second, f):

    if (first == second):
        f.write('Route are the same\n')
    else:
        if( first[0][COL_INDEX['CountManeuver']] == 'Error' or second[0][COL_INDEX['CountManeuver']] == 'Error'):
            f.write('Route containts error')
            return

        if (int(first[0][COL_INDEX['CountManeuver']]) > int(second[0][COL_INDEX['CountManeuver']])):
            f.write('In first route count of maneuvers > then in second route\n')
            firstManeuver = first
            secondManeuver = second

        if (int(first[0][COL_INDEX['CountManeuver']]) < int(second[0][COL_INDEX['CountManeuver']])):
            f.write('In first route count of maneuvers < then in second route\n')
            firstManeuver = second
            secondManeuver = first

        if (int(first[0][COL_INDEX['CountManeuver']]) == int(second[0][COL_INDEX['CountManeuver']])):
            f.write('In first route count of maneuvers = count of second route\n')
            firstManeuver = first
            secondManeuver = second

        k = 0

        #compare origion
        pr = compareInformation(firstManeuver[0][COL_INDEX["Origin"]],secondManeuver[0][COL_INDEX["Origin"]])

        if (pr < 80 ):
            str =  'Origin are not same for maneuver with index \n'
            f.write(str)

        #compare destinition
        pr = compareInformation(firstManeuver[0][COL_INDEX["Destination"]],secondManeuver[0][COL_INDEX["Destination"]])

        if (pr < 80 ):
            str =  'Destination are not same for maneuver with index \n'
            f.write(str)

        #compare Mode
        pr = compareInformation(firstManeuver[0][COL_INDEX["Mode"]],secondManeuver[0][COL_INDEX["Mode"]])

        if (pr < 80 ):
            str =  'Mode are not same for maneuver with index \n'
            f.write(str)

        for row in firstManeuver:

            res = compareManeuver(row, secondManeuver[k],f)

            if (res == True):
                k +=1

def main(argv=None):
    """
    MAIN METHOD
    """
    print ""
    print "*************************************************"
    print "*** BatchTool Routes Comparator               ***"
    print "*************************************************"
    print ""

    if argv is None:
        argv = sys.argv
    progname = argv[0]
    if progname is None:
        progname = "btcomparator_for_route.py"

    if len(argv) < 3:
        print "Parametrers incorrect"
        return 2

    firstFile = argv[1]
    secondFile = argv[2]
    resultFile = argv[3]

    print "Comparision %s vs %s " % (firstFile, secondFile)

    firstTmpFile = 'off_board.csv'
    secondTmpFile = 'on_board.csv'

    shutil.copyfile(firstFile, firstTmpFile)
    shutil.copyfile(secondFile, secondTmpFile)

    f1 = open(firstTmpFile, 'a')
    f2 = open(secondTmpFile, 'a')

    foff = open('off_outputresult.txt', 'a')
    fon = open('on_outputresult.txt', 'a')

    firstCsv = csv.writer(f1)
    secondCsv = csv.writer(f2)
    firstCsv.writerow([-100, -100])
    secondCsv.writerow([-100, -100])
    f1.close()
    f2.close()

    f1 = open(firstTmpFile, 'r')
    f2 = open(secondTmpFile, 'r')

    firstCsv = csv.reader(f1)
    secondCsv = csv.reader(f2)
    f = open(resultFile, 'w')

    firstRoute = []
    secondRoute = []

    k = 1
    for row in firstCsv:

        if ('#CaseId' == row[COL_INDEX["ID"]]):
            continue

        index = int(row[COL_INDEX["ID"]])

        if( k == index ):
            firstRoute.append(row)

        else:
            flag = True
            temp = []
            for secondRow in secondCsv:

                if ('#CaseId' == secondRow[COL_INDEX["ID"]]):
                    continue

                secondIndex = int(secondRow[COL_INDEX["ID"]])

                if (k == secondIndex):
                    secondRoute.append(secondRow)
                else:
                    temp = secondRow
                    break

            str = "\n\n------------------------Compare CaseID: %i--------------------------- \n" % (k)
            f.write(str)
            str = "\n\n------------------------CaseID: %i--------------------------- \n" % (k)
            fon.write(str)
            foff.write(str)
            f.write("Route for offBoard mode:\n")
            routeWriteToFile(firstRoute, f)
            routeWriteToFile(firstRoute, foff)
            f.write("Route for onBoard mode:\n")
            routeWriteToFile(secondRoute, f)
            routeWriteToFile(secondRoute, fon)
            try:
                compareRoute(firstRoute,secondRoute, f)
            except Exception, e:
                f.write("Exception\n")
            firstRoute = []
            secondRoute = []
            k= k+1
            firstRoute.append(row)
            secondRoute.append(temp)

    f.close()
    f1.close()
    f2.close()

if __name__ == '__main__':
    main()

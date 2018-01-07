import os
import sys
import time
import math

"""
Batch Tool Comparator
"""

g_comparePrecision=0.0002
g_ignorePhoneContryCode=True
g_categoryFilter=False

#CaseId,Lat/Lon,Name,Address,AreaName,Airport,Distance,Phones,Categories
g_colCaseId=0
g_colLatLon=1
g_colName=2
g_colAddress=3
g_colAreaName=4
g_colAirport=5
g_colDistance=6
g_colPhones=7
g_colCategories=8

g_categoryDict=[]

def main(argv=None):
    """
    MAIN METHOD
    """
    print ""
    print "*************************************************"
    print "*** BatchTool Comparator                      ***"
    print "*************************************************"
    print ""

    if argv is None:
        argv = sys.argv
    progname = argv[0]
    if progname is None:
        progname = "btcomparator.py"

    if len(argv) < 5:
        print "Usage:\n%s Dictionary InTestsFile OffBoard OnBoard Output" % progname
        return 2

    fileDict = argv[1]
    fileTests = argv[2]
    fileFirst = argv[3]
    fileSecond = argv[4]
    fileResult = argv[5]

    print "Comparision %s vs %s using %s..." % (fileFirst, fileSecond, fileTests)

    # parse dictionary
    #Type/SubType,Name OnBoard,Code OnBoard,Compressed Code OnBoard,Name OffBoard,Compressed Code OffBoard,Server Codes
    fpDict = open(fileDict,"rb")
    for strDict in fpDict:
        paramsIn=parseCSVString(strDict)
        if len(paramsIn):
            buf=[]
            buf.append(paramsIn[1])     # name
            buf.append(paramsIn[3])     # Compressed onboard
            buf.append(paramsIn[5])     # Compressed offboard
            buf.append(paramsIn[6])     # Server codes
            g_categoryDict.append(buf)
    fpDict.close()

    fpTests = open(fileTests,"rb")
    fpSecond = open(fileSecond,"rb")
    fpResult = open(fileResult,"wb")
    fpFirst = open(fileFirst,"rb")

    fpResult.write("#CaseId,Matched,Pos off,Pos on,lat/lon off,lat/lon on,Name off,Name on,Address off,Address on,AreaName off,AreaName on,Airport off,Airport on,Distance off,Distance on,Phones off,Phones On,Categories off,Categories on,\"Query (CaseId,...)\"\n")
    for strTest in fpTests:
        paramsIn=parseCSVString(strTest)
        if len(paramsIn):

            blockFirst=[]
            blockSecond=[]

            fpFirst.seek(0)
            for strFirst in fpFirst:
                paramsFirst=parseCSVString(strFirst)
                if len(paramsFirst):
                    if paramsIn[0]==paramsFirst[0]:
                        blockFirst.append(paramsFirst)

            fpSecond.seek(0)
            for strSecond in fpSecond:
                paramsSecond=parseCSVString(strSecond)
                if len(paramsSecond):
                    if paramsIn[0]==paramsSecond[0]:
                        blockSecond.append(paramsSecond)

            cmpResult=compareBlocks(blockFirst,blockSecond)
            for line in cmpResult:
                for item in line:
                    fpResult.write("\""+item+"\",")

                if line[1]=="TC status":
                    fpResult.write("\n")
                else:
                    fpResult.write("\"")
                    for item in paramsIn:
                        fpResult.write(item+",")
                    fpResult.write("\"\n")

    fpTests.close()
    fpFirst.close()
    fpSecond.close()
    fpResult.close()

    return 0

def compareBlocks(blockA,blockB):
    """
    BlockComparator
    """
    i=0
    j=0
    out=[]
    flagFail=0
    flagSecondBlock=[]
    deletedByCategory=[]
    deletedByCategoryB=[]

    testCaseID="N/A"

    # Get test case id
    if len(blockA):
        testCaseID=blockA[0][0]
    elif len(blockB):
        testCaseID=blockB[0][0]

    if g_categoryFilter:
        # unsupport category filtering
        i=0
        while i < len(blockA):
            if len(blockA[i][g_colCategories]):
                flag=False
                for outCategory in blockA[i][g_colCategories].split("+"):
                    for dictCategory in g_categoryDict:
                        if outCategory in dictCategory[3].split("|"):
                            flag = True
                            break
                    if flag:
                        break
                if not flag:
                    deletedByCategory.append(blockA[i])
                    blockA[i:i+1]=[]
            i=i+1
        # end of unsupport category filtering

    scoreMatrix=[]
    for i in xrange(len(blockA)):
        scoreMatrix.append([])
        for j in xrange(len(blockB)):
            scoreMatrix[i].append(float(0))

    for i in xrange(len(blockA)):
        for j in xrange(len(blockB)):
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffLatLon(blockA[i][g_colLatLon],blockB[j][g_colLatLon]) * 14
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffText(blockA[i][g_colName],blockB[j][g_colName]) * 14
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffText(blockA[i][g_colAddress],blockB[j][g_colAddress]) * 16
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffText(blockA[i][g_colAreaName],blockB[j][g_colAreaName]) * 14
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffText(blockA[i][g_colAirport],blockB[j][g_colAirport]) * 14
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffPhones(blockA[i][g_colPhones],blockB[j][g_colPhones]) * 14
            scoreMatrix[i][j]=scoreMatrix[i][j]+diffCategories(blockA[i][g_colCategories],blockB[j][g_colCategories]) * 14
            ## TODO: distance comparision

    resultsA=[]
    for i in xrange(len(blockA)):
        resultsA.append(-1)

    resultsAScore=[]
    for i in xrange(len(blockA)):
        resultsAScore.append(-1)

    resultsB=range(len(blockB))

    flagPop=True
    while flagPop:
        curMaxVal=-1
        curMaxI=0
        curMaxJ=0
        flagPop=False
        for i in xrange(len(blockA)):
            for j in xrange(len(blockB)):
                if scoreMatrix[i][j]>curMaxVal:
                    curMaxVal=scoreMatrix[i][j]
                    curMaxI=i
                    curMaxJ=j
                    flagPop=True

        #pop value from scoreMatrix
        if flagPop:
            resultsA[curMaxI]=curMaxJ
            resultsB[curMaxJ]=-1
            resultsAScore[curMaxI]=scoreMatrix[curMaxI][curMaxJ]
            for i in xrange(len(blockA)):
                scoreMatrix[i][curMaxJ]=-1
                if i==curMaxI:
                    for j in xrange(len(blockB)):
                        scoreMatrix[i][j]=-1

    summaryRelevance=0
    summaryMismatched=0
    summaryMissedOff=0
    summaryMissedOn=0
    flagRelevanceStop=False

    for i in xrange(len(blockA)):
        if resultsA[i]>=0:
            if not flagRelevanceStop:
                if resultsA[i]==i:
                    summaryRelevance = summaryRelevance + 1
                else:
                    flagRelevanceStop=True

            if resultsAScore[i]<95:
                summaryMismatched = summaryMismatched + 1

            print "|".join(blockA[i])
            print "|".join(blockB[resultsA[i]])
            print ""
            buf=[]
            for j in xrange(len(blockA[i])):
                buf.append(blockA[i][j])
                if j==0:
                    buf.append("%.0f%%" % resultsAScore[i])
                    buf.append("%i" % i)
                    buf.append("%i" % resultsA[i])
                if j>0:
                    if j<len(blockB[resultsA[i]]):
                        buf.append(blockB[resultsA[i]][j])
                    else:
                        buf.append("")
            out.append(buf)
        else:
            summaryMissedOn=summaryMissedOn+1
            print "|".join(blockA[i])
            print "--missed--"
            print ""
            buf=[]
            for j in xrange(len(blockA[i])):
                buf.append(blockA[i][j])
                if j==0:
                    buf.append("missed")    #onboard
                    buf.append("%i" % i)
                    buf.append("")
                if j>0:
                    buf.append("")
            out.append(buf)

    for i in xrange(len(blockB)):
        if resultsB[i]>=0:
            if len(deletedByCategoryB)>=len(deletedByCategory):
                summaryMissedOff=summaryMissedOff+1
                print "--missed--"
                print "|".join(blockB[i])
                print ""
                buf=[]
                for j in xrange(len(blockB[i])):
                    if j==0:
                        buf.append(blockB[i][j])
                        buf.append("missed")    #offboard
                        buf.append("")
                        buf.append("%i" % i)
                    if j>0:
                        buf.append("")
                        buf.append(blockB[i][j])
                out.append(buf)
            else:
                deletedByCategoryB.append(blockB[i])
    summaryAll=len(out)

    if g_categoryFilter:
        for i in xrange(len(deletedByCategory)):
            print "|".join(deletedByCategory[i])
            print "--deleted by category--"
            print
            buf=[]
            for j in xrange(len(deletedByCategory[i])):
                if j==0:
                    buf.append(deletedByCategory[i][j])
                    buf.append("not supported category")
                    buf.append("")
                    buf.append("")
                else:
                    buf.append(deletedByCategory[i][j])
                    if i<len(deletedByCategoryB):
                        buf.append(deletedByCategoryB[i][j])
                    else:
                        buf.append("")
            out.append(buf)

    buf=[]

    if testCaseID!="N/A":
        buf.append(testCaseID)
        buf.append("TC status")
        if summaryAll>0:
            buf.append("Total: %i; Relevance: %.0f%%; Missmatched: %.0f%%; Missed Offboard: %i; Missed OnBoard: %i"%(summaryAll,float(summaryRelevance)/float(summaryAll)*100.0,float(summaryMismatched)/float(summaryAll)*100.0,summaryMissedOff,summaryMissedOn))
        else:
            buf.append("Total: 0; Relevance: 100%%; Missmatched: 0%%; Missed Offboard: 0; Missed OnBoard: 0")
        out.append(buf)
    return out


def diffText(strA,strB):
    if strA==strB:
        return 1.0
    elif strA.lower()==strB.lower():
        return 0.8
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
                    return float(matchedCount) / float(len(wordsA))

                else:
                    for wordA in wordsA:
                        if strBLow.find(wordA)!=-1:
                            matchedCount=matchedCount+1
                    return float(matchedCount) / float(len(wordsB))

        return 0

def diffPhones(phoneA,phoneB):
    if g_ignorePhoneContryCode:
        if phoneA.startswith("()") or phoneB.startswith("()"):
            if phoneB.startswith("1"):
                phoneB=phoneB.replace("1", "", 1)
            if phoneA.startswith("1"):
                phoneA=phoneA.replace("1", "", 1)
            if phoneB.replace("(","").replace(")","") == phoneA.replace("(","").replace(")",""):
                return 0.9
        else:
            if phoneA==phoneB:
                return 1
    else:
        if phoneA==phoneB:
            return 1
    return 0

def diffLatLon(latlonA,latlonB):
    inItemsA=latlonA.split(",")
    inItemsB=latlonB.split(",")
    j=0
    result=0.0
    flagNonFloat=False
    for inItemA in inItemsA:                # Check for lat/lon
        if j<len(inItemsB):
            try:
                floatA=float(inItemA)
                floatB=float(inItemsB[j])
                if floatA>floatB:
                    result=result+floatA-floatB
                else:
                    result=result+floatB-floatA

            except (TypeError, ValueError):
                return 0
        else:
            return 0
        j=j+1
    if result<=g_comparePrecision:
        return 1
    else:
        return g_comparePrecision/result

def diffCategories(categoriesA, categoriesB):
    if g_categoryFilter:
        lenA=len(categoriesA)
        lenB=len(categoriesB)
        if lenA==0 and lenB==0: return 1
        if lenA==0 and lenB!=0 or lenA!=0 and lenB==0: return 0

        categoryMatched=0
        arrA=categoriesA.split("+")
        arrB=categoriesB.split("+")

        #convert onboard to server category
        for i in xrange(len(arrB)):
            flag=False
            for dictCategory in g_categoryDict:
                if arrB[i] == dictCategory[1]:
                    for serverCat in dictCategory[3].split("|"):
                        for itemA in arrA:
                            if itemA==serverCat:
                                flag=True
                                break
                        if flag: break
                    break
            if flag:
                categoryMatched=categoryMatched+1

        return float(categoryMatched)/float(len(arrB))
    else:
        if categoriesA == categoriesB: return 1
        lenA=len(categoriesA)
        lenB=len(categoriesB)
        if lenA==0 and lenB!=0 or lenA!=0 and lenB==0: return 0

        arrA = categoriesA.split("+")
        arrB = categoriesB.split("+")
        lenA=len(arrA)
        lenB=len(arrB)
        matchedCount = 0
        if lenA > lenB:
            for wordB in arrB:
                if categoriesA.find(wordB)!=-1:
                    matchedCount=matchedCount+1
            return float(matchedCount) / float(lenA)
        else:
            for wordA in arrA:
                if categoriesB.find(wordA)!=-1:
                    matchedCount=matchedCount+1
            return float(matchedCount) / float(lenB)


def parseCSVString(inString):
    """
    Parse CSV string
    """
    out=[]
    buf=""
    flagQuote=0

    inString=inString.strip()

    for char in inString:
        if char=="\"":
            if flagQuote==0:                      #open quote?
                flagQuote=1
            elif flagQuote==1:                    #duble quote
                flagQuote=0
                buf=buf+char
            elif flagQuote==2:                    #close quote?
                flagQuote=3
            elif flagQuote==3:                    #duble quote
                flagQuote=2
                buf=buf+char
        elif char==",":
            if flagQuote==0 or flagQuote==3:      #quote closed or no quotes
                flagQuote=0
                if buf=="\"": buf=""
                out.append(buf)
                buf=""
            else:                                 #under quote
                buf=buf+char
                if flagQuote==1: flagQuote=2
        else:
            if flagQuote==1: flagQuote=2
            elif flagQuote==3: flagQuote=0
            buf=buf+char

    if buf=="\"": buf=""
    out.append(buf)
    if out[0][0]=="#": out=[]
    return out


if __name__ == '__main__':
    main()

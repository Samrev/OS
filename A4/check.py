import os
import random
from random import randint
import time
from subprocess import call
import glob
import difflib

random.seed(0)

def createRandomString(n):
    s = ""
    i = 0
    while i < n:
        num = randint(0,3)
        if num == 0:
            s = s+'N'
        if num == 1:
            s = s+'E'
        if num == 2:
            s = s+'S'
        if num == 3:
            s = s+'W'
        i += 1
    return s

def generateTestCases():
    numTestCases = 10
    f = open("Test_Cases.txt", "w")
    f.write("N"+"\n")
    f.write("E"+"\n")
    f.write("S"+"\n")
    f.write("W"+"\n")
    f.write("NSWE"+"\n")
    s = ""
    j = 0
    while j < 10:
        s = s+"N"
        j += 1
    s = s+"W"
    f.write(s+"\n")
    s = s+"ES"
    f.write(s+"\n")

    s = ""
    j = 0
    while j < 10:
        s = s+"E"
        j += 1
    s = s+"S"
    f.write(s+"\n")
    s = s+"NW"
    f.write(s+"\n")

    i = 0
    while i < numTestCases:
        n = randint(10,50)
        s = createRandomString(n)
        f.write(s+"\n")
        i += 1
    f.close()

def check(file,TestString,TC_No):
    f = open(file,"r")
    lines = f.readlines()
    isNorth = 0
    isSouth = 0
    isEast = 0
    isWest = 0
    cntNorth = 0
    cntEast = 0
    cntWest = 0
    cntSouth = 0
    starvationTime = 0
    i = 0
    isDeadlockDetected = False
    while i < len(lines):
        if lines[i][0:-1] == "Train Arrived at the lane from the North direction":
            if isNorth == 1:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from North Arrives, check Line No. : "+str(i))
                return False
            isNorth = 1
            cntNorth += 1
            if cntNorth == 1:
                if i+1 > starvationTime:
                    starvationTime = i+1
        elif lines[i][0:-1] == "Train Exited the lane from the North direction":
            if isNorth == 0:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from North Exits, check Line No. : "+str(i))
                return False
            if isNorth == 1 and isEast == 1 and isSouth == 1 and isWest == 1 and isDeadlockDetected == False:
                print("Test Case "+str(TC_No)+" failed !! - Deadlock not detected, check Line No. : "+str(i))
                return False
            isNorth = 0
            isDeadlockDetected = False

        elif lines[i][0:-1] == "Train Arrived at the lane from the South direction":
            if isSouth == 1:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from South Arrives, check Line No. : "+str(i))
                return False
            isSouth = 1
            cntSouth += 1
            if cntSouth == 1:
                if i+1 > starvationTime:
                    starvationTime = i+1
        elif lines[i][0:-1] == "Train Exited the lane from the South direction":
            if isSouth == 0:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from South Exits, check Line No. : "+str(i))
                return False
            if isNorth == 1 and isEast == 1 and isSouth == 1 and isWest == 1 and isDeadlockDetected == False:
                print("Test Case "+str(TC_No)+" failed !! - Deadlock not detected, check Line No. : "+str(i))
                return False
            isSouth = 0
            isDeadlockDetected = False

        elif lines[i][0:-1] == "Train Arrived at the lane from the East direction":
            if isEast == 1:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from East Arrives, check Line No. : "+str(i))
                return False
            isEast = 1
            cntEast += 1
            if cntEast == 1:
                if i+1 > starvationTime:
                    starvationTime = i+1
        elif lines[i][0:-1] == "Train Exited the lane from the East direction":
            if isEast == 0:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from East Exits, check Line No. : "+str(i))
                return False
            if isNorth == 1 and isEast == 1 and isSouth == 1 and isWest == 1 and isDeadlockDetected == False:
                print("Test Case "+str(TC_No)+" failed !! - Deadlock not detected, check Line No. : "+str(i))
                return False
            isEast = 0
            isDeadlockDetected = False

        elif lines[i][0:-1] == "Train Arrived at the lane from the West direction":
            if isWest == 1:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from West Arrives, check Line No. : "+str(i))
                return False
            isWest = 1
            cntWest += 1
            if cntWest == 1:
                if i+1 > starvationTime:
                    starvationTime = i+1
        elif lines[i][0:-1] == "Train Exited the lane from the West direction":
            if isWest == 0:
                print("Test Case "+str(TC_No)+" failed !! - 2 Trains from West Exits, check Line No. : "+str(i))
                return False
            if isNorth == 1 and isEast == 1 and isSouth == 1 and isWest == 1 and isDeadlockDetected == False:
                print("Test Case "+str(TC_No)+" failed !! - Deadlock not detected, check Line No. : "+str(i))
                return False
            isWest = 0
            isDeadlockDetected = False

        elif lines[i][0:-1] == "Deadlock detected. Resolving deadlock...":
            if isNorth != 1 or isEast != 1 or isSouth != 1 or isWest != 1:
                print("Test Case "+str(TC_No)+" failed !! : Deadlock not there but shown, check Line No. : "+str(i))
                return False
            isDeadlockDetected = True
        i += 1

    if isNorth == 1 or isEast == 1 or isWest == 1 or isSouth == 1:
        print("Test Case "+str(TC_No)+" failed !! : All trains doesn't exit")
        return False

    i = 0
    while i < len(TestString):
        if TestString[i] == 'N':
            cntNorth -= 1
        if TestString[i] == 'S':
            cntSouth -= 1
        if TestString[i] == 'E':
            cntEast -= 1
        if TestString[i] == 'W':
            cntWest -= 1
        i += 1

    if cntNorth != 0 or cntEast != 0 or cntWest != 0 or cntSouth != 0:
        print("Test Case "+str(TC_No)+" failed !! : Counts of trains not same")
        return False

    starvationTime = starvationTime/(2*len(TestString))
    if starvationTime > 0.75:
        print("Test Case "+str(TC_No)+" failed !! : Starvation Time more than 0.75")
        return False

    return True

if os.path.isdir("Outputs") == False:
    os.mkdir("Outputs")

generateTestCases()

f = open("Test_Cases.txt", "r")
lines = f.readlines()
i = 0
TCFailed = []
while i < len(lines):
    s = lines[i][0:-1]
    print("Running Line : "+str(i)+" ... \r", end="")
    fileName = os.path.join("Outputs","Output_Line_No_"+str(i)+".txt")
    fileOutput = open(fileName, "w")
    call(["./main.exe", s], stdout=fileOutput)
    fileOutput.close()
    if check(fileName,s,i) == False:
        TCFailed.append(i)
    i += 1

f.close()

if len(TCFailed) == 0:
    print("All Test Cases Passed !!")
else:
    print("Test Cases Failed are : ")
    for i in TCFailed:
        print("Line No : "+str(i))

import os
import random
from random import randint
import time
from subprocess import call
import glob
import difflib

def compare(file1, file2):
    f1 = open(file1, "r")
    f2 = open(file2, "r")
    lines1 = f1.readlines()
    lines2 = f2.readlines()
    if len(lines1) != len(lines2):
        return True
    i = 0
    while i < len(lines1):
        if lines1[i] != lines2[i]:
            return True
        i += 1
    return False

def convertIntToHexa(x):
    s = list("0xffffffff")
    i = 9
    while i >= 2:
        num = int(x)%16
        if num <= 9:
            s[i] = chr(num+48)
        else:
            s[i] = chr((num-10)+97)
        x = int(x/16)
        i -= 1
    s = ''.join(s)
    return s

random.seed(0)

PageSize = 4*1024
NumPages = 1024*1024

areTestCasesGenerated = True
if os.path.isdir("Test_Cases") == False:
    os.mkdir("Test_Cases")
    areTestCasesGenerated = False
if os.path.isdir("Test_Cases_Outputs") == False:
    os.mkdir("Test_Cases_Outputs")

if areTestCasesGenerated == False:
    start = time.time()
    print("Generating Test Cases ... ")

    n = []
    l = 10
    u = 100
    for i in range(5):
        num = randint(l,u-1)
        n.append(num)
        l = 10*l
        u = 10*u

    n.append(1e7)

    SpatialLocalityFileNames = ["Spacial_Locality_TraceFileLength_10_100.txt", "Spacial_Locality_TraceFileLength_100_1000.txt",
                                "Spacial_Locality_TraceFileLength_1000_10000.txt","Spacial_Locality_TraceFileLength_10000_100000.txt",
                                "Spacial_Locality_TraceFileLength_100000_1000000.txt","Spacial_Locality_TraceFileLength_1000000.txt"]
    TemporalLocalityFileNames = ["Temporal_Locality_TraceFileLength_10_100.txt", "Temporal_Locality_TraceFileLength_100_1000.txt",
                                 "Temporal_Locality_TraceFileLength_1000_10000.txt","Temporal_Locality_TraceFileLength_10000_100000.txt",
                                 "Temporal_Locality_TraceFileLength_100000_1000000.txt","Temporal_Locality_TraceFileLength_1000000.txt"]
    RandomFileNames = ["Random_TraceFileLength_10_100.txt", "Random_TraceFileLength_100_1000.txt",
                       "Random_TraceFileLength_1000_10000.txt","Random_TraceFileLength_10000_100000.txt",
                       "Random_TraceFileLength_100000_1000000.txt","Random_TraceFileLength_1000000.txt"]

    # TC : Spatial Locality

    for i in range(6):
        fileName = os.path.join("Test_Cases",SpatialLocalityFileNames[i])
        f = open(fileName, "w")
        traceLength = n[i]
        j = 0
        while j < traceLength:
            VPN = randint(0, NumPages-1)
            perc = int(randint(10,25)*traceLength/100)
            k = 0
            while j < traceLength and k < perc:
                isWrite = randint(0, 1)
                num = randint(VPN*PageSize, (VPN+1)*PageSize-1)
                s = convertIntToHexa(num)
                c = "W"
                if isWrite == 0:
                    c = "R"
                f.write(s+"  "+c+"\n")
                j += 1
                k += 1
        f.close()

    # TC : Temporal Locality

    for i in range(6):
        fileName = os.path.join("Test_Cases",TemporalLocalityFileNames[i])
        f = open(fileName, "w")
        traceLength = n[i]
        numImportantPages = randint(3,7)
        importantPages = []
        for q in range(numImportantPages):
            importantPages.append(randint(0, NumPages-1))
        j = 0
        while j < traceLength:
            perc = randint(0,9)
            if perc <= 2:
                VPN = randint(0,NumPages-1)
            else:
                VPN = importantPages[randint(0,numImportantPages-1)]
            num = randint(VPN*PageSize, (VPN+1)*PageSize-1)
            isWrite = randint(0,1)
            s = convertIntToHexa(num)
            c = "W"
            if isWrite == 0:
                c = "R"
            f.write(s+"  "+c+"\n")
            j += 1
        f.close()

    # TC : Random

    for i in range(5):
        fileName = os.path.join("Test_Cases",RandomFileNames[i])
        f = open(fileName, "w")
        traceLength = n[i]
        j = 0
        while j < traceLength:
            VPN = randint(0,NumPages-1)
            num = randint(VPN*PageSize, (VPN+1)*PageSize-1)
            isWrite = randint(0,1)
            s = convertIntToHexa(num)
            c = "W"
            if isWrite == 0:
                c = "R"
            f.write(s+"  "+c+"\n")
            j += 1
        f.close()

    print("Test Cases Generated !!")
    end = time.time()

    totalTimeTaken = end-start
    min = int(totalTimeTaken/60)
    sec = int(totalTimeTaken)%60
    print("Time Taken to generate Test Cases : "+str(min)+"m "+str(sec)+"s")

TestCasesFiles = []
os.chdir("Test_Cases")
for file in glob.glob("*.txt"):
    TestCasesFiles.append(os.path.join("Test_Cases",file))

os.chdir("..")
SFrames = [5,10,50,100]
RFrames = [100,200,500,1000]
strategy = ["OPT","FIFO","CLOCK","LRU","RANDOM"]
print("Generating Test Outputs...")

start = time.time()
for file in TestCasesFiles:
    file2 = file[11:-4]
    for i in range(4):
        frames = SFrames[i]
        if file2[0] == 'R':
            frames = RFrames[i]
        for k in range(5):
            newFile = file2+"_Frames_"+str(frames)+"_Strategy_"+strategy[k]+".txt"
            newFile = os.path.join("Test_Cases_Outputs", newFile)
            fileOutput = open(newFile, "w")
            call(["./frames",file,str(frames),strategy[k]],stdout=fileOutput)

print("Outputs of Test Case Generated !!")
end = time.time()
totalTimeTaken = end-start
min = int(totalTimeTaken/60)
sec = int(totalTimeTaken)%60
print("Time Taken to generate Test Outputs : "+str(min)+"m "+str(sec)+"s")

print("Test Cases Checking Starts...")
UsersTestCasesOutputsFiles = []
os.chdir("Test_Cases_Outputs")
for file in glob.glob("*.txt"):
    UsersTestCasesOutputsFiles.append(os.path.join("Test_Cases_Outputs",file))

os.chdir("..")
RaunakTestCasesOutputsFiles = []
os.chdir("Raunak_Test_Cases_Outputs")
for file in glob.glob("*.txt"):
    RaunakTestCasesOutputsFiles.append(os.path.join("Raunak_Test_Cases_Outputs",file))

os.chdir("..")
RaunakTestCasesOutputsFiles.sort()
UsersTestCasesOutputsFiles.sort()

if len(UsersTestCasesOutputsFiles) != len(RaunakTestCasesOutputsFiles):
    print("All Test Cases not generated !!")
    exit()

TestCasesFailed = []
for i in range(len(RaunakTestCasesOutputsFiles)):
    if compare(RaunakTestCasesOutputsFiles[i],UsersTestCasesOutputsFiles[i]):
        TestCasesFailed.append(UsersTestCasesOutputsFiles)

if len(TestCasesFailed) > 0:
    print("Total Test Cases Failed are : "+str(len(TestCasesFailed))+" and they are : ")
    for TC in TestCasesFailed:
        print(TC)
        break

else:
    print("All Test Cases Passed !!")















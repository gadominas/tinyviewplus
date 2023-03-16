//
//  tvppHeatService.cpp
//  ripView
//
//  Created by Gediminas Siutilas on 2023-03-15.
//

#include "tvppHeatService.h"
#include "ofApp.h"
#include <iostream>

using namespace std;

ofFile heatResultsFile;
tvpPilotHeatStat pilotHeatStat[MAX_HEAT_SLOTS];

float calcMinSumPairs(float arr[], int n) {
   float minSum = 999;
    
   for (int i=0; i< (n - 1); i++)
     if(minSum > (arr[i] + arr[i + 1]))
       minSum = (arr[i] + arr[i + 1]);
   
   return minSum;
}

float getFastedConsecutiveLaps(string pilotName, float minLapTime){
    int heatSlotIndex = getPilotHeatSlotIndex(pilotName);
    float filteredLaps[ARAP_MAX_RLAPS];
    
    int addedIndex = 0;
    for (int i = 0; i < pilotHeatStat[heatSlotIndex].totalLaps + 1; i++) {
        if( pilotHeatStat[heatSlotIndex].lapHistLapTime[i] > minLapTime){
            filteredLaps[addedIndex] = pilotHeatStat[heatSlotIndex].lapHistLapTime[i];
            addedIndex++;
        }
    }
    
    if( addedIndex > 0 ){
        return calcMinSumPairs(filteredLaps, addedIndex);
    } else {
        99.0f;
    }
}

heatResults getHeatResults(float minLapTime){
    int totalPilots = 0;
    heatResults hr;
    
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if(pilotHeatStat[i].pilotName != "") {
            hr.heatResultPerPilot[hr.totalPilotsInHeat] =
                pilotHeatStat[i].pilotName + " >> " +
            getLapStr(pilotHeatStat[i].bestLap) + "s / 2flap: " + getLapStr(getFastedConsecutiveLaps(pilotHeatStat[i].pilotName, minLapTime)) + "s";
            
            hr.totalPilotsInHeat += 1;
        }
    }
    
    return hr;
}

int getPilotHeatSlotIndex(string pilotName) {
    // locate pilotheat slot by pilot name
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if(pilotHeatStat[i].pilotName == pilotName) {
            return i;
        }
    }
    
    // in case no slot were found by pilot name, look for a 1st unused one
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if(pilotHeatStat[i].pilotName == "") {
            return i;
        }
    }
    
    return 0;
}

void clearHeatResults(){
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        pilotHeatStat[i].pilotName = "";
        pilotHeatStat[i].bestLap = BEST_LAP_REST_VALUE;
        pilotHeatStat[i].totalLaps = 0;
        
        for(int a = 0; a < ARAP_MAX_RLAPS; a++){
            pilotHeatStat[i].lapHistLapTime[a] = LAP_HISTORY_LAP_TIME_RESET_VALUE;
        }
    }
}

void mergeHistLapTimeValues(float source[], int sourceOffset, float destination[], int destinationOffset, bool useStartGate){
    int offset = (useStartGate == true ? 1 : 0);
    
    string dumpValues = "useStartGate:" + ofToString(useStartGate) + " [round] >>>> ";
    for (int i = 0; i < destinationOffset; i++) {
        dumpValues += " , " + ofToString(destination[i]);
    }
    
    ofLog() << dumpValues;
    
    for (int i = offset; i < destinationOffset + offset; i++) {
        source[sourceOffset+i] = destination[i];
    }
    
    dumpValues = "useStartGate:" + ofToString(useStartGate) + " [total] >>>> ";
    
    for (int i = 0; i < (sourceOffset+destinationOffset); i++) {
        dumpValues += " , " + ofToString(source[i]);
    }
    
    ofLog() << dumpValues;
}

void updateHeatStats(tvpCamView camView[], int cameraNum, int minLapTime, bool useStartGate){
    for (int i = 0; i < cameraNum; i++) {
        string pilotName = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        int pos = camView[i].racePosition;
        int lps = camView[i].totalLaps - (useStartGate == true ? 1 : 0);
        float blap = getBestLap(i);
        
        // if best lap is less than min lap time do not count that
        if( blap <= minLapTime ) {
            continue;
        }
        
        int heatIndex = getPilotHeatSlotIndex(pilotName);
        mergeHistLapTimeValues(pilotHeatStat[heatIndex].lapHistLapTime,
                               pilotHeatStat[heatIndex].totalLaps,
                               camView[i].lapHistLapTime,
                               lps + 1,
                               useStartGate);
        
        pilotHeatStat[heatIndex].pilotName = pilotName;
        pilotHeatStat[heatIndex].totalLaps += lps;
        pilotHeatStat[heatIndex].bestLap = min(blap, pilotHeatStat[heatIndex].bestLap);
        
        ofLog() << pilotHeatStat[heatIndex].pilotName << " new best lap: " << pilotHeatStat[heatIndex].bestLap << " total laps: " << pilotHeatStat[heatIndex].totalLaps;
    }
}

float getBestHeatLapTime(string pilotName){
    int heatSlotIndex = getPilotHeatSlotIndex(pilotName);
    return pilotHeatStat[heatSlotIndex].bestLap;
}

void fwriteRaceHeatResult() {
    if (isRecordedLaps() == false) {
        return;
    }
    
    string sep = "  ";
    string timestamp = ofGetTimestampString();
    string newline;
#ifdef TARGET_WIN32
    newline = "\r\n";
#else /* TARGET_WIN32 */
    newline = "\n";
#endif /* TARGET_WIN32 */
    string strsumm = timestamp + newline + newline;
    strsumm += "- heat results -" + newline;
    
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if( pilotHeatStat[i].pilotName != "" ){
            strsumm += pilotHeatStat[i].pilotName + sep; // Name
            strsumm += " best lap time: " + ofToString(pilotHeatStat[i].bestLap);
            strsumm += " total laps: " + ofToString(pilotHeatStat[i].totalLaps);
            strsumm += newline;
            
            for (int o = 0; o < pilotHeatStat[i].totalLaps + 1; o++) {
                strsumm += ofToString(pilotHeatStat[i].lapHistLapTime[o]);
                strsumm += newline;
            }
            
            strsumm += newline + " -------- ";
        }
    }
    
    strsumm += newline;

    // write to file
    heatResultsFile.open(ARAP_RESULT_DIR + timestamp + "_heat.txt" , ofFile::WriteOnly);
    heatResultsFile << (strsumm);
    heatResultsFile.close();
    // copy to clipboard
    ofSetClipboardString(strsumm);
}

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
    
   for (int i=0; i< (n - 2); i++)
     if(minSum > (arr[i] + arr[i + 1] + arr[i + 2]))
       minSum = (arr[i] + arr[i + 1] + arr[i + 2]);
   
   return minSum;
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

float getFastedConsecutiveLaps(string pilotName, float minLapTime, int consecLapCount){
    tvpPilotHeatStat phStat = pilotHeatStat[getPilotHeatSlotIndex(pilotName)];
    
    float filteredLaps[ARAP_MAX_RLAPS];
    float bestHeatsLapTime = BEST_LAP_REST_VALUE;

    for (int i = 0; i < phStat.totalHeats; i++) {
        if( phStat.heats[i].totalLaps >= consecLapCount ){
            bestHeatsLapTime = min(bestHeatsLapTime,  calcMinSumPairs(phStat.heats[i].histLapTime, phStat.heats[i].totalLaps));
        } else {
            continue;
        }
    }

    if( bestHeatsLapTime != BEST_LAP_REST_VALUE ){
        return bestHeatsLapTime;
    } else {
        return BEST_LAP_REST_VALUE;
    }
}

heatResults getHeatResults(float minLapTime, int consecLapCount){
    int totalPilots = 0;
    heatResults hr;
    
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if(pilotHeatStat[i].pilotName != "") {
            hr.pilotName[hr.totalPilotsInHeat] = pilotHeatStat[i].pilotName;
            hr.bestLapTime[hr.totalPilotsInHeat] = getLapStr(pilotHeatStat[i].overallBestLap);
            
            float consLaps = getFastedConsecutiveLaps(pilotHeatStat[i].pilotName, minLapTime, consecLapCount);
            if( consLaps == BEST_LAP_REST_VALUE){
                hr.bestConsTime[hr.totalPilotsInHeat] = "n/a";
            } else {
                hr.bestConsTime[hr.totalPilotsInHeat] = getLapStr(consLaps);
            }
            
            hr.totalHeats[hr.totalPilotsInHeat] = ofToString(pilotHeatStat[i].totalHeats);

            hr.totalPilotsInHeat += 1;
        }
    }
    
    return hr;
}

void clearHeatResults(){
    // clear all pilot heat slots
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        pilotHeatStat[i].pilotName = "";
        pilotHeatStat[i].overallBestLap = BEST_LAP_REST_VALUE;
        pilotHeatStat[i].totalHeats = 0;
        
        // clear all pilot heat records
        for(int a = 0; a < COMBINED_RECORD_MAX_HISTORY; a++){
            pilotHeatStat[i].heats[a].totalLaps = 0;
            pilotHeatStat[i].heats[a].bestLapTime = BEST_LAP_REST_VALUE;
            
            // clear all lap timing within a pilot heat record
            for(int l = 0; l < LAP_MAX_RECORDS; l++){
                pilotHeatStat[i].heats[a].histLapTime[l] = LAP_HISTORY_LAP_TIME_RESET_VALUE;
        }
    }
    }
}

void copyRoundStatsToRepo(int pilotIndex, float destination[], int totalHeatLaps, bool useStartGate){
    tvpPilotHeatStat* phHeatRecord = &pilotHeatStat[pilotIndex];
    tvHeatRecord* heatRecord = &phHeatRecord->heats[phHeatRecord->totalHeats];
    
    int offset = (useStartGate == true ? 1 : 0);
    heatRecord->totalLaps = totalHeatLaps - offset;
    
    for(int i = 0; i < totalHeatLaps; i++){
        // copy lap times from a particular round to a pilot record
        heatRecord->histLapTime[i] = destination[i+offset];
        
        // calculate best lap per round
        heatRecord->bestLapTime = min(heatRecord->bestLapTime, heatRecord->histLapTime[i]);
    }
    
}
    
float calcBestLapThroughAllRounds(tvpPilotHeatStat pilotHeatStat, float minLapTime){
    float blap = BEST_LAP_REST_VALUE;
    
    for (int i = 0; i < pilotHeatStat.totalHeats + 1; i++) {
        int totalHeatLaps = pilotHeatStat.heats[i].totalLaps;
        
        for (int l = 0; l < totalHeatLaps + 1 ; l++) {
            float histLapTime = pilotHeatStat.heats[i].histLapTime[l];
            
            if( histLapTime > minLapTime ) {
                blap = min(blap, pilotHeatStat.heats[i].histLapTime[l]);
            }
        }
    }
    
    return blap;
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
        
        copyRoundStatsToRepo(heatIndex, camView[i].lapHistLapTime, lps + 1, useStartGate);
        pilotHeatStat[heatIndex].pilotName = pilotName;
        
        // increase a round counter for a particular pilot
        pilotHeatStat[heatIndex].totalHeats += 1;
        
        // calculate best lap through all rounds
        pilotHeatStat[heatIndex].overallBestLap = calcBestLapThroughAllRounds(pilotHeatStat[heatIndex], minLapTime);
        
        ofLog() << pilotHeatStat[heatIndex].pilotName << " overall best lap: " << pilotHeatStat[heatIndex].overallBestLap << " total heats: " << pilotHeatStat[heatIndex].totalHeats;
    }
}

float getBestHeatLapTime(string pilotName){
    return pilotHeatStat[getPilotHeatSlotIndex(pilotName)].overallBestLap;
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
    strsumm += "----- heats results for all pilots ------" + newline;
    
    for (int i = 0; i < MAX_HEAT_SLOTS; i++) {
        if( pilotHeatStat[i].pilotName != "" ){
            strsumm += "Pilot name:" + pilotHeatStat[i].pilotName + newline; // Name
            strsumm += "Overall best lap time: " + getLapStr(pilotHeatStat[i].overallBestLap) + newline;
            strsumm += "Total heats: " + ofToString(pilotHeatStat[i].totalHeats) + newline;
            
            for (int h = 0; h < pilotHeatStat[i].totalHeats; h++) {
                strsumm += "   heat: " + ofToString(h+1) + " results -" + newline;
                strsumm += "     * best lap within a heat: " + getLapStr(pilotHeatStat[i].heats[h].bestLapTime) + newline;
                strsumm += "     * total laps within a heat: " + ofToString(pilotHeatStat[i].heats[h].totalLaps) + newline;
                
                strsumm += "     heat laps results -" + newline;
                for(int l = 0; l < pilotHeatStat[i].heats[h].totalLaps + 1; l++){
                    strsumm += newline + "       lap: " + ofToString(l+1) + " -> " + getLapStr(pilotHeatStat[i].heats[h].histLapTime[l]);
                }
                
                strsumm += newline + newline;
            }
            
            strsumm += newline + " -------- ";
        }
    }
    strsumm += newline;

    // write to file
    heatResultsFile.open(ARAP_RESULT_HEATS_DIR + timestamp + "_heats.txt" , ofFile::WriteOnly);
    heatResultsFile << (strsumm);
    heatResultsFile.close();
    // copy to clipboard
    ofSetClipboardString(strsumm);
}

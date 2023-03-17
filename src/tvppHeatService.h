#pragma once
//
//  tvppHeatService.hpp
//  ripView
//
//  Created by Gediminas Siutilas on 2023-03-15.
//
#include <stdio.h>
#include "ofApp.h"

#define MAX_HEAT_SLOTS 99
#define BEST_LAP_REST_VALUE 9999
#define LAP_HISTORY_LAP_TIME_RESET_VALUE 9999
#define COMBINED_RECORD_MAX_HISTORY 99
#define LAP_MAX_RECORDS 99

#define ARAP_RESULT_HEATS_DIR "results/"

class tvHeatRecord {
public:
    int totalLaps = 0;
    float bestLapTime = BEST_LAP_REST_VALUE;
    float histLapTime[LAP_MAX_RECORDS + 1];
};

class tvpPilotHeatStat {
public:
    string pilotName = "";
    int totalHeats = 0;
    float overallBestLap = BEST_LAP_REST_VALUE;
    tvHeatRecord heats[COMBINED_RECORD_MAX_HISTORY + 1];
};


class heatResults {
public:
    int totalPilotsInHeat = 0;
    string pilotName[ARAP_MAX_RLAPS + 1];
    string bestLapTime[ARAP_MAX_RLAPS + 1];
    string bestConsTime[ARAP_MAX_RLAPS + 1];
    string totalHeats[ARAP_MAX_RLAPS + 1];
};


/**
  Get total pilots participating in a heat session
 */
heatResults getHeatResults(float, int);

/**
 Calculate fasted two consecutive laps
 */
float getFastedConsecutiveLaps(string, float, int);

/**
 Flush out heat results upon race finishes
 */
void fwriteRaceHeatResult();

/**
 Get the best heat lap time results by pilot name
 */
float getBestHeatLapTime(string);

/**
 Get pilot heat data slot id by pilot name
 */
int getPilotHeatSlotIndex(string);

/**
 Update heat stats after race is finishes recalculating best lap time and merging lap times into lapHistLapTime.
 */
void updateHeatStats(tvpCamView[], int, int, bool);

/**
 Clear heat results triggerd by the user
 */
void clearHeatResults();

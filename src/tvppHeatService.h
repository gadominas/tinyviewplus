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

class tvpPilotHeatStat {
public:
    string pilotName = "";
    int totalLaps = 0;
    float bestLap = BEST_LAP_REST_VALUE;
    float lapHistLapTime[ARAP_MAX_RLAPS + 1];
};


class heatResults {
public:
    int totalPilotsInHeat = 0;
    string heatResultPerPilot[ARAP_MAX_RLAPS + 1];
};


/**
  Get total pilots participating in a heat session
 */
heatResults getHeatResults(float);

/**
 Calculate fasted two consecutive laps
 */
float getFastedConsecutiveLaps(string, float);

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

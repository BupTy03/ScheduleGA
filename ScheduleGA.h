#pragma once
#include "ScheduleCommon.h"
#include "ScheduleIndividual.h"

#include <vector>
#include <chrono>


struct ScheduleGAStatistics
{
   std::chrono::milliseconds Time;
};


struct ScheduleGAParams
{
    int IndividualsCount = 0;
    int IterationsCount = 0;
    int SelectionCount = 0;
    int CrossoverCount = 0;
    int MutationChance = 0;
};


class ScheduleGA
{
public:
    ScheduleGA();
    explicit ScheduleGA(const ScheduleGAParams& params);

    static ScheduleGAParams DefaultParams();
    const ScheduleGAParams& Params() const { return params_; }

    ScheduleGAStatistics Start(const ScheduleData& scheduleData);
    const std::vector<ScheduleIndividual>& Individuals() const;

private:
    ScheduleGAParams params_;
    std::vector<ScheduleIndividual> individuals_;
};

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
    std::size_t IndividualsCount = 0;
    std::size_t IterationsCount = 0;
    std::size_t SelectionCount = 0;
    std::size_t CrossoverCount = 0;
    std::size_t MutationChance = 0;
};


class ScheduleGA
{
public:
    ScheduleGA();
    explicit ScheduleGA(const ScheduleGAParams& params);

    static ScheduleGAParams DefaultParams();
    const ScheduleGAParams& Params() const;

    ScheduleGAStatistics Start(const std::vector<SubjectRequest>& requests);
    const std::vector<ScheduleIndividual>& Individuals() const;

private:
    ScheduleGAParams params_;
    std::vector<ScheduleIndividual> individuals_;
};

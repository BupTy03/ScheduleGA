#pragma once
#include "ScheduleCommon.h"
#include "ScheduleIndividual.h"

#include <vector>
#include <chrono>


struct ScheduleGAStatistics
{
   std::chrono::milliseconds Time;
};

class ScheduleGA
{
public:
    ScheduleGA();

    ScheduleGA& IndividualsCount(std::size_t individuals);
    std::size_t IndividualsCount() const;

    ScheduleGA& IterationsCount(std::size_t iterations);
    std::size_t IterationsCount() const;

    ScheduleGA& SelectionCount(std::size_t selectionCount);
    std::size_t SelectionCount() const;

    ScheduleGA& CrossoverCount(std::size_t crossoverCount);
    std::size_t CrossoverCount() const;

    ScheduleGA& MutationChance(std::size_t mutationChance);
    std::size_t MutationChance() const;

    const ScheduleIndividual& Best() const;
    ScheduleGAStatistics Start(const std::vector<SubjectRequest>& requests);

private:
    std::size_t individualsCount_;
    std::size_t iterationsCount_;
    std::size_t selectionCount_;
    std::size_t crossoverCount_;
    std::size_t mutationChance_;
    std::vector<ScheduleIndividual> individuals_;
};

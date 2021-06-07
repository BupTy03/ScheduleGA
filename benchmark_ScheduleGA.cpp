#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include "ScheduleGA.h"

#include <iostream>


TEST_CASE("ScheduleGA benchmark")
{
    std::random_device randomDevice;

    ScheduleDataGenerator generator{randomDevice, 1, 5, 0, 7};
    const ScheduleData data{generator.GenerateSubjectRequests(200)};

    ScheduleGA algo{ScheduleGAParams{
        .IndividualsCount = 100,
        .IterationsCount = 1000,
        .SelectionCount = 36,
        .CrossoverCount = 22,
        .MutationChance = 49
    }};

    BENCHMARK("ScheduleGA")
    {
        algo.Start(data);
        return algo.Individuals().front().Evaluate();
    };
}


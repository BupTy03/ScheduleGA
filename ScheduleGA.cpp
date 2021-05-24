#include "ScheduleGA.h"

#include <iostream>
#include <cassert>
#include <exception>
#include <execution>


ScheduleGA::ScheduleGA()
    : params_(ScheduleGA::DefaultParams())
    , individuals_()
{
}

ScheduleGA::ScheduleGA(const ScheduleGAParams& params)
    : params_(params)
{
    if(params_.IndividualsCount == 0)
        throw std::invalid_argument("Invalid IndividualsCount option: must be greater than zero");

    if(params_.SelectionCount >= params_.IndividualsCount)
        throw std::invalid_argument("Invalid SelectionCount option: must be less than IndividualsCount");

    if(params_.MutationChance > 100)
        throw std::invalid_argument("Invalid MutationChance option: must be in range [0, 100]");
}

ScheduleGAParams ScheduleGA::DefaultParams()
{
    ScheduleGAParams result;
    result.IndividualsCount = 1000;
    result.IterationsCount = 1100;
    result.SelectionCount = 360;
    result.CrossoverCount = 220;
    result.MutationChance = 49;
    return result;
}

const ScheduleGAParams& ScheduleGA::Params() const
{
    return params_;
}

ScheduleGAStatistics ScheduleGA::Start(const std::vector<SubjectRequest>& requests)
{
    std::random_device randomDevice;
    const ScheduleIndividual firstIndividual(randomDevice, &requests);
    firstIndividual.Evaluate();

    individuals_.clear();
    individuals_.resize(params_.IndividualsCount, firstIndividual);

    std::mt19937 randGen(randomDevice());

    std::uniform_int_distribution<std::size_t> selectionBestDist(0, params_.SelectionCount - 1);
    std::uniform_int_distribution<std::size_t> individualsDist(0, individuals_.size() - 1);

    const auto beginTime = std::chrono::steady_clock::now();

    ScheduleGAStatistics result;
    for(std::size_t iteration = 0; iteration < params_.IterationsCount; ++iteration)
    {
        // mutate
        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(), ScheduleIndividualMutator(params_.MutationChance));

        // select best
        std::nth_element(individuals_.begin(), individuals_.begin() + params_.SelectionCount, individuals_.end(), ScheduleIndividualLess());

        //std::cout << "Iteration: " << iteration << "; Best: " << std::min_element(individuals_.begin(), individuals_.begin() + SelectionCount(), ScheduleIndividualLess())->Evaluate() << '\n';

        // crossover
        for(std::size_t i = 0; i < params_.CrossoverCount; ++i)
        {
            ScheduleIndividual& firstInd = individuals_.at(selectionBestDist(randGen));
            ScheduleIndividual& secondInd = individuals_.at(individualsDist(randGen));
            firstInd.Crossover(secondInd);
        }

        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(), ScheduleIndividualEvaluator());

        // natural selection
        std::nth_element(individuals_.begin(), individuals_.end() - params_.SelectionCount, individuals_.end(), ScheduleIndividualLess());
        std::copy_n(individuals_.begin(), params_.SelectionCount, individuals_.end() - params_.SelectionCount);
    }

    std::sort(individuals_.begin(), individuals_.end(), ScheduleIndividualLess());
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
    return result;
}

const std::vector<ScheduleIndividual>& ScheduleGA::Individuals() const
{
    assert(std::is_sorted(individuals_.begin(), individuals_.end(), ScheduleIndividualLess()));
    return individuals_;
}

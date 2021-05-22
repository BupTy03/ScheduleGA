#include "ScheduleGA.h"

#include <cassert>
#include <execution>


ScheduleGA::ScheduleGA()
    : individualsCount_(1000)
    , iterationsCount_(1000)
    , selectionCount_(125)
    , crossoverCount_(25)
    , mutationChance_(45)
    , individuals_()
{
}

ScheduleGA& ScheduleGA::IndividualsCount(std::size_t individuals) { individualsCount_ = individuals; return *this; }
std::size_t ScheduleGA::IndividualsCount() const { return individualsCount_; }

ScheduleGA& ScheduleGA::IterationsCount(std::size_t iterations) { iterationsCount_ = iterations; return *this; }
std::size_t ScheduleGA::IterationsCount() const { return iterationsCount_; }

ScheduleGA& ScheduleGA::SelectionCount(std::size_t selectionCount) { selectionCount_ = selectionCount; return *this; }
std::size_t ScheduleGA::SelectionCount() const { return selectionCount_; }

ScheduleGA& ScheduleGA::CrossoverCount(std::size_t crossoverCount) { crossoverCount_ = crossoverCount; return *this; }
std::size_t ScheduleGA::CrossoverCount() const { return crossoverCount_; }

ScheduleGA& ScheduleGA::MutationChance(std::size_t mutationChance) 
{ 
    assert(mutationChance >= 0 && mutationChance <= 100);
    mutationChance_ = mutationChance;
    return *this;
}
std::size_t ScheduleGA::MutationChance() const { return mutationChance_; }

const ScheduleIndividual& ScheduleGA::Best() const { return individuals_.front(); }

ScheduleGAStatistics ScheduleGA::Start(const std::vector<SubjectRequest>& requests)
{
    std::random_device randomDevice;
    individuals_.clear();
    individuals_.reserve(individualsCount_);
    for(std::size_t i = 0; i < individualsCount_; ++i)
        individuals_.emplace_back(randomDevice, &requests);

    assert(selectionCount_ < individuals_.size());
    assert(crossoverCount_ < selectionCount_);

    std::mt19937 randGen(randomDevice());

    std::uniform_int_distribution<std::size_t> selectionBestDist(0, SelectionCount() - 1);
    std::uniform_int_distribution<std::size_t> individualsDist(0, individuals_.size() - 1);

    const auto beginTime = std::chrono::steady_clock::now();

    ScheduleGAStatistics result;
    std::size_t prevEvaluated = std::numeric_limits<std::size_t>::max();
    for(std::size_t iteration = 0; iteration < IterationsCount(); ++iteration)
    {
        // mutate
        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(), [this](ScheduleIndividual& individual)
        {
            if(individual.MutationProbability() <= MutationChance())
            {
                individual.Mutate();
                individual.Evaluate();
            }
        });

        // select best
        std::partial_sort(individuals_.begin(), individuals_.begin() + SelectionCount(), individuals_.end(), ScheduleIndividualLess());
        const std::size_t currentBestEvaluated = individuals_.front().Evaluate();

        //std::cout << "Iteration: " << iteration << "; Best: " << currentBestEvaluated << '\n';
        if(currentBestEvaluated < prevEvaluated)
            result.Iterations = iteration;

        prevEvaluated = currentBestEvaluated;

        // crossover
        for(std::size_t i = 0; i < CrossoverCount(); ++i)
        {
            auto& firstInd = individuals_.at(selectionBestDist(randGen));
            auto& secondInd = individuals_.at(individualsDist(randGen));

            firstInd.Crossover(secondInd);
            firstInd.Evaluate();
            secondInd.Evaluate();
        }

        // natural selection
        std::nth_element(individuals_.begin(), individuals_.end() - SelectionCount(), individuals_.end(), ScheduleIndividualLess());
        std::sort(individuals_.end() - SelectionCount(), individuals_.end(), ScheduleIndividualLess());
        std::copy_n(individuals_.begin(), SelectionCount(), individuals_.end() - SelectionCount());
    }

    std::sort(individuals_.begin(), individuals_.end(), ScheduleIndividualLess());
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
    return result;
}

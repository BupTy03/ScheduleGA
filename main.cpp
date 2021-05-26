#include "ScheduleGA.h"

#include <iostream>
#include <array>
#include <execution>
#include <algorithm>


static void FindOptimalIterationsCount(const std::vector<SubjectRequest>& requests, std::ostream& os = std::cout)
{
	std::array<std::size_t, 1000> results = {};
	constexpr std::size_t step = 100;

	for(std::size_t iterationsCount = step; iterationsCount < results.size() * step; iterationsCount += step)
	{
		os << "Iterations count: " << iterationsCount;

		std::array<std::size_t, 100> attempts = {};
		for(auto& a : attempts)
		{
			ScheduleGAParams params;
			params.IndividualsCount = 100;
			params.IterationsCount = iterationsCount;
			params.SelectionCount = 36;
			params.CrossoverCount = 22;
			params.MutationChance = 49;

			ScheduleGA algo(params);
			algo.Start(requests, {});
			a = algo.Individuals().front().Evaluate();
		}

		std::sort(attempts.begin(), attempts.end());

		os << ", min: " << attempts.front();
		os << ", max: " << attempts.back();
		os << ", median: " << attempts.at(attempts.size() / 2);
		os << ", avg: " << static_cast<double>(std::accumulate(attempts.begin(), attempts.end(), 0)) / 
			static_cast<double>(attempts.size()) << std::endl;
	}
}

struct OptimalParams
{
	double Evaluated = std::numeric_limits<double>::max();
	std::size_t SelectionCount = 0;
	std::size_t CrossoverCount = 0;
	std::size_t MutationChance = 0;
};

static std::ostream& operator<<(std::ostream& os, const OptimalParams& params)
{
	os << "eval: " << params.Evaluated << 
		", sel: " << params.SelectionCount << 
		", cross: " << params.CrossoverCount << 
		", mut: " << params.MutationChance;

	return os;
}

static void FindOptimalParams(const std::vector<SubjectRequest>& requests, std::ostream& os = std::cout)
{
	OptimalParams bestMinParams;
	OptimalParams bestMaxParams;
	OptimalParams bestMedianParams;
	OptimalParams bestAvgParams;

	constexpr auto MIN_SELECTION_COUNT = 20;
	constexpr auto MAX_SELECTION_COUNT = 50;

	constexpr auto MIN_CROSSOVER_COUNT = 5;
	constexpr auto MAX_CROSSOVER_COUNT = 25;

	constexpr auto MIN_MUTATION_CHANCE = 30;
	constexpr auto MAX_MUTATION_CHANCE = 50;

	for(std::size_t selectionCount = MIN_SELECTION_COUNT; selectionCount <= MAX_SELECTION_COUNT; ++selectionCount)
	{
		for(std::size_t crossoverCount = MIN_CROSSOVER_COUNT; crossoverCount <= MAX_CROSSOVER_COUNT; ++crossoverCount)
		{
			for(std::size_t mutationChance = MIN_MUTATION_CHANCE; mutationChance <= MAX_MUTATION_CHANCE; ++mutationChance)
			{
				std::array<std::size_t, 100> attempts = {};
				std::for_each(std::execution::par, attempts.begin(), attempts.end(), [&](auto& a)
				{
					ScheduleGAParams params;
					params.IndividualsCount = 100;
					params.IterationsCount = 100;
					params.SelectionCount = selectionCount;
					params.CrossoverCount = crossoverCount;
					params.MutationChance = mutationChance;

					ScheduleGA algo(params);
					algo.Start(requests, {});
					a = algo.Individuals().front().Evaluate();
				});

				std::sort(attempts.begin(), attempts.end());

				const std::size_t minVal = attempts.front();
				const std::size_t maxVal = attempts.back();
				const std::size_t medianVal = attempts.at(attempts.size() / 2);
				const double avgVal = static_cast<double>(std::accumulate(attempts.begin(), attempts.end(), 0)) / 
					static_cast<double>(attempts.size());

				// best minimum
				if(minVal < bestMinParams.Evaluated)
				{
					bestMinParams.Evaluated = minVal;
					bestMinParams.SelectionCount = selectionCount;
					bestMinParams.CrossoverCount = crossoverCount;
					bestMinParams.MutationChance = mutationChance;
				}

				// best maximum
				if(maxVal < bestMaxParams.Evaluated)
				{
					bestMaxParams.Evaluated = maxVal;
					bestMaxParams.SelectionCount = selectionCount;
					bestMaxParams.CrossoverCount = crossoverCount;
					bestMaxParams.MutationChance = mutationChance;
				}

				// best median
				if(medianVal < bestMedianParams.Evaluated)
				{
					bestMedianParams.Evaluated = medianVal;
					bestMedianParams.SelectionCount = selectionCount;
					bestMedianParams.CrossoverCount = crossoverCount;
					bestMedianParams.MutationChance = mutationChance;
				}

				// best avg
				if(avgVal < bestAvgParams.Evaluated)
				{
					bestAvgParams.Evaluated = avgVal;
					bestAvgParams.SelectionCount = selectionCount;
					bestAvgParams.CrossoverCount = crossoverCount;
					bestAvgParams.MutationChance = mutationChance;
				}

				os << "[sel: " << selectionCount << ", cross: " << crossoverCount << ", mut: " << mutationChance << ']' << 
					" min: " << minVal << ", max: " << maxVal << 
					", median: " << medianVal << ", avg: " << avgVal << '\n';
				}
		}
	}

	os << "Best min [" << bestMinParams << "]\n";
	os << "Best max [" << bestMaxParams << "]\n";
	os << "Best median [" << bestMedianParams << "]\n";
	os << "Best avg [" << bestAvgParams << "]" << std::endl;
}


int main()
{
	std::vector<bool> weekDays = {false, true, true, true, false, false, false, true, true, true, true, false};

	std::vector<SubjectRequest> requests = {
		SubjectRequest(0, 0, 1, weekDays, {1}, {ClassroomAddress(0, 101), ClassroomAddress(0, 102)}),
		SubjectRequest(1, 2, 2, weekDays, {1, 2}, {ClassroomAddress(0, 104), ClassroomAddress(0, 105)}),
		SubjectRequest(2, 4, 4, weekDays, {3}, {ClassroomAddress(0, 101)}),
		SubjectRequest(3, 6, 3, weekDays, {4}, {ClassroomAddress(0, 105), ClassroomAddress(0, 107)}),
		SubjectRequest(4, 0, 1, weekDays, {1}, {ClassroomAddress(0, 101), ClassroomAddress(0, 103)}),
		SubjectRequest(5, 2, 2, weekDays, {1, 2}, {ClassroomAddress(1, 104), ClassroomAddress(1, 105)}),
		SubjectRequest(6, 4, 4, weekDays, {3}, {ClassroomAddress(0, 101), ClassroomAddress(0, 110)}),
		SubjectRequest(7, 6, 1, weekDays, {4}, {ClassroomAddress(0, 105), ClassroomAddress(0, 107)}),
		SubjectRequest(8, 0, 1, weekDays, {1}, {ClassroomAddress(0, 101), ClassroomAddress(0, 103)}),
		SubjectRequest(9, 2, 3, weekDays, {1, 2}, {ClassroomAddress(0, 104), ClassroomAddress(0, 105)}),
		SubjectRequest(10, 4, 2, weekDays, {3}, {ClassroomAddress(0, 101), ClassroomAddress(0, 120)}),
		SubjectRequest(11, 6, 1, weekDays, {4}, {ClassroomAddress(0, 105), ClassroomAddress(0, 109)})
	};

	std::vector<SubjectWithAddress> lockedLessons = {
		SubjectWithAddress(0, 0),
		SubjectWithAddress(1, 1),
		SubjectWithAddress(2, 2),
		SubjectWithAddress(3, 3),
		SubjectWithAddress(4, 4)
	};

	//FindOptimalParams(requests);
	//FindOptimalIterationsCount(requests);

	ScheduleGAParams params;
	params.IndividualsCount = 100;
    params.IterationsCount = 1100;
    params.SelectionCount = 36;
    params.CrossoverCount = 22;
    params.MutationChance = 49;

	ScheduleGA algo(params);
	const auto stat = algo.Start(requests, lockedLessons);

	const auto& bestIndividual = algo.Individuals().front();
	Print(bestIndividual);
	std::cout << "Best: " << bestIndividual.Evaluate() << '\n';
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.Time).count() << "ms.\n";
	std::cout.flush();
	return 0;
}

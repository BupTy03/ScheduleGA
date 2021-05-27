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
			algo.Start(ScheduleData(requests, {}));
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
					algo.Start(ScheduleData(requests, {}));
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
		SubjectRequest(0, 0, 1, weekDays, {1}, {{0, 101}, {0, 102}}),
		SubjectRequest(1, 2, 2, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(2, 4, 4, weekDays, {3}, {{0, 101}}),
		SubjectRequest(3, 6, 3, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(4, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(5, 2, 2, weekDays, {1, 2}, {{1, 104}, {1, 105}}),
		SubjectRequest(6, 4, 4, weekDays, {3}, {{0, 101}, {0, 110}}),
		SubjectRequest(7, 6, 1, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(8, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(9, 2, 3, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(10, 4, 2, weekDays, {3}, {{0, 101}, {0, 120}}),
		SubjectRequest(11, 6, 1, weekDays, {4}, {{0, 105}, {0, 109}}),

		SubjectRequest(12, 0, 1, weekDays, {1}, {{0, 101}, {0, 102}}),
		SubjectRequest(13, 2, 2, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(14, 4, 4, weekDays, {3}, {{0, 101}}),
		SubjectRequest(15, 6, 3, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(16, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(17, 2, 2, weekDays, {1, 2}, {{1, 104}, {1, 105}}),
		SubjectRequest(18, 4, 4, weekDays, {3}, {{0, 101}, {0, 110}}),
		SubjectRequest(19, 6, 1, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(20, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(21, 2, 3, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(22, 4, 2, weekDays, {3}, {{0, 101}, {0, 120}}),
		SubjectRequest(23, 6, 1, weekDays, {4}, {{0, 105}, {0, 109}}),

		SubjectRequest(24, 0, 1, weekDays, {1}, {{0, 101}, {0, 102}}),
		SubjectRequest(25, 2, 2, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(27, 4, 4, weekDays, {3}, {{0, 101}}),
		SubjectRequest(28, 6, 3, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(29, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(30, 2, 2, weekDays, {1, 2}, {{1, 104}, {1, 105}}),
		SubjectRequest(31, 4, 4, weekDays, {3}, {{0, 101}, {0, 110}}),
		SubjectRequest(32, 6, 1, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(33, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(34, 2, 3, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(35, 4, 2, weekDays, {3}, {{0, 101}, {0, 120}}),
		SubjectRequest(36, 6, 1, weekDays, {4}, {{0, 105}, {0, 109}}),

		SubjectRequest(37, 0, 1, weekDays, {1}, {{0, 101}, {0, 102}}),
		SubjectRequest(38, 2, 2, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(39, 4, 4, weekDays, {3}, {{0, 101}}),
		SubjectRequest(40, 6, 3, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(41, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(42, 2, 2, weekDays, {1, 2}, {{1, 104}, {1, 105}}),
		SubjectRequest(43, 4, 4, weekDays, {3}, {{0, 101}, {0, 110}}),
		SubjectRequest(44, 6, 1, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(45, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(46, 2, 3, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(47, 4, 2, weekDays, {3}, {{0, 101}, {0, 120}}),
		SubjectRequest(48, 6, 1, weekDays, {4}, {{0, 105}, {0, 109}}),

		SubjectRequest(49, 0, 1, weekDays, {1}, {{0, 101}, {0, 102}}),
		SubjectRequest(50, 2, 2, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(51, 4, 4, weekDays, {3}, {{0, 101}}),
		SubjectRequest(52, 6, 3, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(53, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(54, 2, 2, weekDays, {1, 2}, {{1, 104}, {1, 105}}),
		SubjectRequest(55, 4, 4, weekDays, {3}, {{0, 101}, {0, 110}}),
		SubjectRequest(56, 6, 1, weekDays, {4}, {{0, 105}, {0, 107}}),
		SubjectRequest(57, 0, 1, weekDays, {1}, {{0, 101}, {0, 103}}),
		SubjectRequest(58, 2, 3, weekDays, {1, 2}, {{0, 104}, {0, 105}}),
		SubjectRequest(59, 4, 2, weekDays, {3}, {{0, 101}, {0, 120}}),
		SubjectRequest(60, 6, 1, weekDays, {4}, {{0, 105}, {0, 109}})
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
	params.IndividualsCount = 1000;
    params.IterationsCount = 1100;
    params.SelectionCount = 360;
    params.CrossoverCount = 220;
    params.MutationChance = 49;

	ScheduleGA algo(params);
	const ScheduleData data(requests, lockedLessons);
	const auto stat = algo.Start(data);

	const auto& bestIndividual = algo.Individuals().front();
	Print(bestIndividual, data);
	std::cout << "Best: " << bestIndividual.Evaluate() << '\n';
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.Time).count() << "ms.\n";
	std::cout.flush();
	return 0;
}

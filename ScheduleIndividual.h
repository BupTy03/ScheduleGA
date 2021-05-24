#pragma once
#include "ScheduleCommon.h"
#include "LinearAllocator.h"

#include <vector>
#include <random>
#include <tuple>


bool GroupsOrProfessorsOrClassroomsIntersects(const std::vector<SubjectRequest>& requests,
                                              const std::vector<std::size_t>& lessons,
                                              const std::vector<ClassroomAddress>& classrooms,
                                              std::size_t currentRequest,
                                              std::size_t currentLesson);

bool GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                  const std::vector<std::size_t>& lessons,
                                  std::size_t currentRequest,
                                  std::size_t currentLesson);

bool ClassroomsIntersects(const std::vector<std::size_t>& lessons,
                          const std::vector<ClassroomAddress>& classrooms,
                          std::size_t currentLesson,
                          const ClassroomAddress& currentClassroom);

void InitChromosomes(std::vector<std::size_t>& lessons,
                     std::vector<ClassroomAddress>& classrooms,
                     const std::vector<SubjectRequest>& requests,
                     std::size_t requestIndex);

std::tuple<std::vector<std::size_t>, std::vector<ClassroomAddress>> InitChromosomes(const std::vector<SubjectRequest>& requests);

std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const std::vector<SubjectRequest>& requests,
                             const std::vector<std::size_t>& lessons,
                             const std::vector<ClassroomAddress>& classrooms);


class ScheduleIndividual
{

    static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024;
    static constexpr std::size_t NOT_EVALUATED = std::numeric_limits<std::size_t>::max();
public:
    explicit ScheduleIndividual(std::random_device& randomDevice,
                                const std::vector<SubjectRequest>* pRequests);
    void swap(ScheduleIndividual& other) noexcept;

    ScheduleIndividual(const ScheduleIndividual& other);
    ScheduleIndividual& operator=(const ScheduleIndividual& other);

    ScheduleIndividual(ScheduleIndividual&& other) noexcept;
    ScheduleIndividual& operator=(ScheduleIndividual&& other) noexcept;

    const std::vector<SubjectRequest>& Requests() const;
    const std::vector<ClassroomAddress>& Classrooms() const;
    const std::vector<std::size_t>& Lessons() const;

    std::size_t MutationProbability() const;
    void Mutate();
    std::size_t Evaluate() const;
    void Crossover(ScheduleIndividual& other);

private:
    void ChangeClassroom(std::size_t requestIndex);
    void ChangeLesson(std::size_t requestIndex);

private:
    const std::vector<SubjectRequest>* pRequests_;
    mutable std::size_t evaluatedValue_;

    std::vector<ClassroomAddress> classrooms_;
    std::vector<std::size_t> lessons_;

    mutable std::vector<std::uint8_t> buffer_;
    mutable std::mt19937 randomGenerator_;
};

struct ScheduleIndividualLess
{
    bool operator()(const ScheduleIndividual& lhs, const ScheduleIndividual& rhs) const
    {
        return lhs.Evaluate() < rhs.Evaluate();
    }
};

struct ScheduleIndividualEvaluator
{
    void operator()(ScheduleIndividual& individual) const
    {
        individual.Evaluate();
    }
};

struct ScheduleIndividualMutator
{
    explicit ScheduleIndividualMutator(std::size_t mutationChance)
        : MutationChance(mutationChance)
    { }

    void operator()(ScheduleIndividual& individual) const
    {
        if(individual.MutationProbability() <= MutationChance)
        {
            individual.Mutate();
            individual.Evaluate();
        }
    }

    std::size_t MutationChance;
};


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs);

bool ReadyToCrossover(const ScheduleIndividual& first, 
                      const ScheduleIndividual& second,
                      std::size_t requestIndex);

void Print(const ScheduleIndividual& individ);

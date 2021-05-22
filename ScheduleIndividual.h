#pragma once
#include "ScheduleCommon.h"
#include <vector>
#include <random>


class ScheduleIndividual
{
   static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024;
public:
   explicit ScheduleIndividual(std::random_device& randomDevice,
                               const std::vector<SubjectRequest>* pRequests);
   void swap(ScheduleIndividual& other) noexcept;

   ScheduleIndividual(const ScheduleIndividual& other);
   ScheduleIndividual& operator=(const ScheduleIndividual& other);

   ScheduleIndividual(ScheduleIndividual&& other) noexcept;
   ScheduleIndividual& operator=(ScheduleIndividual&& other) noexcept;

   const std::vector<ClassroomAddress>& Classrooms() const;
   const std::vector<std::size_t>& Lessons() const;

   std::size_t MutationProbability() const;
   void Mutate();
   std::size_t Evaluate() const;
   void Crossover(ScheduleIndividual& other, std::size_t requestIndex);

private:
   bool GroupsOrProfessorsIntersects(std::size_t currentRequest,
                                     std::size_t currentLesson) const;

   bool ClassroomsIntersects(std::size_t currentLesson,
                             const ClassroomAddress& currentClassroom) const;

   void Init(std::size_t requestIndex);
   void Change();
   void ChooseClassroom(std::size_t requestIndex);
   void ChooseLesson(std::size_t requestIndex);

private:
   mutable bool evaluated_;
   mutable std::size_t evaluatedValue_;
   std::vector<ClassroomAddress> classrooms_;
   std::vector<std::size_t> lessons_;

   const std::vector<SubjectRequest>* pRequests_;
   mutable std::mt19937 randomGenerator_;
   mutable std::vector<std::uint8_t> buffer_;
};


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs);
void Print(const ScheduleIndividual& individ, const std::vector<SubjectRequest>& requests);

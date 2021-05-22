#include "ScheduleIndividual.h"
#include "LinearAllocator.h"
#include "utils.h"

#include <array>
#include <cassert>
#include <iostream>


constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();


ScheduleIndividual::ScheduleIndividual(std::random_device& randomDevice,
                                       const std::vector<SubjectRequest>* pRequests)
    : evaluated_(false)
    , evaluatedValue_(std::numeric_limits<std::size_t>::max())
    , classrooms_(pRequests->size(), ClassroomAddress::NoClassroom())
    , lessons_(pRequests->size(), std::numeric_limits<std::size_t>::max())
    , pRequests_(pRequests)
    , randomGenerator_(randomDevice())
    , buffer_(DEFAULT_BUFFER_SIZE)
{
    assert(pRequests_ != nullptr);
    for(std::size_t i = 0; i < pRequests_->size(); ++i)
        Init(i);
}

void ScheduleIndividual::swap(ScheduleIndividual& other) noexcept
{
    std::swap(evaluated_, other.evaluated_);
    std::swap(evaluatedValue_, other.evaluatedValue_);
    std::swap(classrooms_, other.classrooms_);
    std::swap(lessons_, other.lessons_);
}

ScheduleIndividual::ScheduleIndividual(const ScheduleIndividual& other)
    : evaluated_(other.evaluated_)
    , evaluatedValue_(other.evaluatedValue_)
    , classrooms_(other.classrooms_)
    , lessons_(other.lessons_)
    , pRequests_(other.pRequests_)
    , randomGenerator_(other.randomGenerator_)
    , buffer_(DEFAULT_BUFFER_SIZE)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(const ScheduleIndividual& other)
{
    ScheduleIndividual tmp(other);
    tmp.swap(*this);
    return *this;
}

ScheduleIndividual::ScheduleIndividual(ScheduleIndividual&& other) noexcept
    : evaluated_(other.evaluated_)
    , evaluatedValue_(other.evaluatedValue_)
    , classrooms_(std::move(other.classrooms_))
    , lessons_(std::move(other.lessons_))
    , pRequests_(other.pRequests_)
    , randomGenerator_(other.randomGenerator_)
    , buffer_(DEFAULT_BUFFER_SIZE)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(ScheduleIndividual&& other) noexcept
{
    other.swap(*this);
    return *this;
}

const std::vector<ClassroomAddress>& ScheduleIndividual::Classrooms() const { return classrooms_; }
const std::vector<std::size_t>& ScheduleIndividual::Lessons() const { return lessons_; }

std::size_t ScheduleIndividual::MutationProbability() const
{
    std::uniform_int_distribution<std::size_t> mutateDistrib(0, 100);
    return mutateDistrib(randomGenerator_);
}

void ScheduleIndividual::Mutate()
{
    assert(pRequests_->size() == classrooms_.size());
    assert(pRequests_->size() == lessons_.size());

    evaluated_ = false;
    Change();
}

std::size_t ScheduleIndividual::Evaluate() const
{
    if(evaluated_)
        return evaluatedValue_;

    evaluated_ = true;
    evaluatedValue_ = 0;

    LinearAllocatorBufferSpan bufferSpan(buffer_.data(), buffer_.size());
    {
    using IntPair = std::pair<std::size_t, std::size_t>;
    using MapPair = std::pair<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>>;
    using BuilingPair = std::pair<std::size_t, std::array<std::size_t, MAX_LESSONS_PER_DAY>>;

    using ComplexityMap = SortedMap<std::size_t, std::size_t, LinearAllocator<IntPair>>;
    using WindowsMap = SortedMap<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>, LinearAllocator<MapPair>>;
    using BuildingsMap = SortedMap<std::size_t, std::array<std::size_t, MAX_LESSONS_PER_DAY>, LinearAllocator<BuilingPair>>;

    std::array<ComplexityMap, DAYS_IN_SCHEDULE> dayComplexity{
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
        ComplexityMap(LinearAllocator<IntPair>(&bufferSpan))
    };

    std::array<WindowsMap, DAYS_IN_SCHEDULE> dayWindows{
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan))
    };

    std::array<WindowsMap, DAYS_IN_SCHEDULE> professorsDayWindows{
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
        WindowsMap(LinearAllocator<MapPair>(&bufferSpan))
    };

    std::array<BuildingsMap, DAYS_IN_SCHEDULE> buildingsInDay{
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan))
    };

    for(std::size_t r = 0; r < lessons_.size(); ++r)
    {
        const auto& request = pRequests_->at(r);
        const std::size_t lesson = lessons_[r];
        if(lesson == NO_LESSON)
        {
            evaluatedValue_ += 100;
            continue;
        }

        const std::size_t day = lesson / MAX_LESSONS_PER_DAY;
        const std::size_t lessonInDay = lesson % MAX_LESSONS_PER_DAY;

        professorsDayWindows[day][request.Professor()][lessonInDay] = true;
        for(std::size_t group : request.Groups())
        {
            dayComplexity[day][group] += (lessonInDay * request.Complexity());
            dayWindows[day][group][lessonInDay] = true;

            auto& buildingsDay = buildingsInDay[day];
            auto it = buildingsDay.lower_bound(group);
            if(it == buildingsDay.end() || it->first != group)
            {
                std::array<std::size_t, MAX_LESSONS_PER_DAY> emptyBuildings;
                emptyBuildings.fill(NO_BUILDING);
                it = buildingsDay.emplace_hint(it, group, emptyBuildings);
            }

            if(classrooms_[r] == ClassroomAddress::NoClassroom())
            {
                evaluatedValue_ += 100;
                continue;
            }

            it->second[lessonInDay] = classrooms_[r].Building;
        }
    }
      
    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
    {
        // evaluating summary complexity of day for group
        for(auto&& p : dayComplexity[d])
        evaluatedValue_ = std::max(evaluatedValue_, p.second);

        // evaluating summary lessons gaps for groups
        for(auto&& p : dayWindows[d])
        {
        std::size_t prevLesson = 0;
        for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
        {
            if(p.second.at(lessonNum))
            {
                const std::size_t lessonsGap = lessonNum - prevLesson;
                prevLesson = lessonNum;

                if(lessonsGap > 1)
                    evaluatedValue_ += lessonsGap * 3;
            }
        }
        }

        // evaluating summary lessons gaps for professors
        for(auto&& p : professorsDayWindows[d])
        {
        std::size_t prevLesson = 0;
        for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
        {
            if(p.second.at(lessonNum))
            {
                const std::size_t lessonsGap = lessonNum - prevLesson;
                prevLesson = lessonNum;

                if(lessonsGap > 1)
                    evaluatedValue_ += lessonsGap * 2;
            }
        }
        }

        // evaluating count transitions between buildings per day
        for(auto&& p : buildingsInDay[d])
        {
        std::size_t prevBuilding = NO_BUILDING;
        for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
        {
            const auto currentBuilding = p.second.at(lessonNum);
            if( !(currentBuilding == NO_BUILDING || prevBuilding == NO_BUILDING || currentBuilding == prevBuilding) )
                evaluatedValue_ += 64;

            prevBuilding = currentBuilding;
        }
        }
    }

    }

    buffer_.resize(std::max(bufferSpan.peak, buffer_.size()));
    return evaluatedValue_;
}

void ScheduleIndividual::Crossover(ScheduleIndividual& other,
                                   std::size_t requestIndex)
{
    assert(!pRequests_->empty());
    assert(pRequests_->size() == lessons_.size());
    assert(classrooms_.size() == lessons_.size());
    assert(other.classrooms_.size() == other.lessons_.size());
    assert(other.lessons_.size() == lessons_.size());

    if(ClassroomsIntersects(other.lessons_.at(requestIndex), other.classrooms_.at(requestIndex)) ||
        other.ClassroomsIntersects(lessons_.at(requestIndex), classrooms_.at(requestIndex)))
        return;

    if(GroupsOrProfessorsIntersects(requestIndex, other.lessons_.at(requestIndex)) ||
        other.GroupsOrProfessorsIntersects(requestIndex, lessons_.at(requestIndex)))
        return;

    evaluated_ = false;
    other.evaluated_ = false;

    std::swap(classrooms_.at(requestIndex), other.classrooms_.at(requestIndex));
    std::swap(lessons_.at(requestIndex), other.lessons_.at(requestIndex));
}

bool ScheduleIndividual::GroupsOrProfessorsIntersects(std::size_t currentRequest,
                                                      std::size_t currentLesson) const
{
    const auto& thisRequest = pRequests_->at(currentRequest);
    auto it = std::find(lessons_.begin(), lessons_.end(), currentLesson);
    while(it != lessons_.end())
    {
        const std::size_t requestIndex = std::distance(lessons_.begin(), it);
        const auto& otherRequest = pRequests_->at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || set_intersects(thisRequest.Groups(), otherRequest.Groups()))
        return true;

        it = std::find(std::next(it), lessons_.end(), currentLesson);
    }

    return false;
}

bool ScheduleIndividual::ClassroomsIntersects(std::size_t currentLesson,
                                              const ClassroomAddress& currentClassroom) const
{
    auto it = std::find(classrooms_.begin(), classrooms_.end(), currentClassroom);
    while(it != classrooms_.end())
    {
        const std::size_t requestIndex = std::distance(classrooms_.begin(), it);
        const std::size_t otherLesson = lessons_.at(requestIndex);
        if(currentLesson == otherLesson)
        return true;

        it = std::find(std::next(it), classrooms_.end(), currentClassroom);
    }

    return false;
}

void ScheduleIndividual::Init(std::size_t requestIndex)
{
    assert(pRequests_->size() == classrooms_.size());
    assert(pRequests_->size() == lessons_.size());

    const auto& request = pRequests_->at(requestIndex);
    const auto& classrooms = request.Classrooms();

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
        if(!request.RequestedWeekDay(day))
            continue;

        const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
        if(!GroupsOrProfessorsIntersects(requestIndex, scheduleLesson))
        {
            for(auto&& classroom : classrooms)
            {
                if(!ClassroomsIntersects(scheduleLesson, classroom))
                {
                    classrooms_.at(requestIndex) = classroom;
                    lessons_.at(requestIndex) = scheduleLesson;
                    return;
                }
            }
        }
        }
    }

    assert(false);
}

void ScheduleIndividual::Change()
{
    assert(pRequests_->size() == classrooms_.size());
    assert(pRequests_->size() == lessons_.size());

    std::uniform_int_distribution<std::size_t> distrib(0, pRequests_->size() - 1);
    const std::size_t requestIndex = distrib(randomGenerator_);
    ChooseClassroom(requestIndex);
    ChooseLesson(requestIndex);
}

void ScheduleIndividual::ChooseClassroom(std::size_t requestIndex)
{
    const auto& request = pRequests_->at(requestIndex);
    const auto& classrooms = request.Classrooms();

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() && ClassroomsIntersects(lessons_.at(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));
        ++chooseClassroomTry;
    }

    if(chooseClassroomTry < classrooms.size())
        classrooms_.at(requestIndex) = scheduleClassroom;
}

void ScheduleIndividual::ChooseLesson(std::size_t requestIndex)
{
    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
    std::size_t scheduleLesson = lessonsDistrib(randomGenerator_);

    const auto& request = pRequests_->at(requestIndex);

    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT && 
        (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) || 
            ClassroomsIntersects(scheduleLesson, classrooms_.at(requestIndex)) || 
            GroupsOrProfessorsIntersects(requestIndex, scheduleLesson)))
    {
        scheduleLesson = lessonsDistrib(randomGenerator_);
        ++chooseLessonTry;
    }

    if(chooseLessonTry < MAX_LESSONS_COUNT)
        lessons_.at(requestIndex) = scheduleLesson;
}


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs)
{
    lhs.swap(rhs);
}

void Print(const ScheduleIndividual& individ, const std::vector<SubjectRequest>& requests)
{
   const auto& lessons = individ.Lessons();
   const auto& classrooms = individ.Classrooms();

   for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
   {
      std::cout << "Lesson " << l << ": ";

      auto it = std::find(lessons.begin(), lessons.end(), l);
      if(it == lessons.end())
      {
         std::cout << '-';
      }
      else
      {
         while(it != lessons.end())
         { 
            const std::size_t r = std::distance(lessons.begin(), it);
            const auto& request = requests.at(r);
            std::cout << "[s:" << r << ", p:" << request.Professor() << ", c:(" << classrooms.at(r).Building << ", " << classrooms.at(r).Classroom << ")]";

            it = std::find(std::next(it), lessons.end(), l);
         }
      }

      std::cout << '\n';
   }

   std::cout.flush();
}

#include "ScheduleIndividual.h"
#include "LinearAllocator.h"
#include "utils.h"

#include <array>
#include <cassert>
#include <iostream>


constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();


bool GroupsOrProfessorsOrClassroomsIntersects(const std::vector<SubjectRequest>& requests,
                                              const std::vector<std::size_t>& lessons,
                                              const std::vector<ClassroomAddress>& classrooms,
                                              std::size_t currentRequest,
                                              std::size_t currentLesson)
{
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons.begin(), lessons.end(), currentLesson);
    while(it != lessons.end())
    {
        const std::size_t requestIndex = std::distance(lessons.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || 
           classrooms.at(currentRequest) == classrooms.at(requestIndex) || 
           set_intersects(thisRequest.Groups(), otherRequest.Groups()))
            return true;

        it = std::find(std::next(it), lessons.end(), currentLesson);
    }

    return false;
}

bool GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                  const std::vector<std::size_t>& lessons,
                                  std::size_t currentRequest, 
                                  std::size_t currentLesson)
{
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons.begin(), lessons.end(), currentLesson);
    while(it != lessons.end())
    {
        const std::size_t requestIndex = std::distance(lessons.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || set_intersects(thisRequest.Groups(), otherRequest.Groups()))
            return true;

        it = std::find(std::next(it), lessons.end(), currentLesson);
    }

    return false;
}

bool ClassroomsIntersects(const std::vector<std::size_t>& lessons,
                          const std::vector<ClassroomAddress>& classrooms,
                          std::size_t currentLesson,
                          const ClassroomAddress& currentClassroom)
{
    auto it = std::find(classrooms.begin(), classrooms.end(), currentClassroom);
    while(it != classrooms.end())
    {
        const std::size_t requestIndex = std::distance(classrooms.begin(), it);
        const std::size_t otherLesson = lessons.at(requestIndex);
        if(currentLesson == otherLesson)
            return true;

        it = std::find(std::next(it), classrooms.end(), currentClassroom);
    }

    return false;
}

bool LessonIsLocked(const std::vector<SubjectWithAddress>& lockedLessons,
                    std::size_t lesson)
{
    assert(std::is_sorted(lockedLessons.begin(), lockedLessons.end(), SubjectWithAddressLess()));
    auto it = std::lower_bound(lockedLessons.begin(), lockedLessons.end(), lesson, SubjectWithAddressLess());
    return it != lockedLessons.end() && it->Address == lesson;
}

bool RequestHasLockedLesson(const std::vector<SubjectWithAddress>& lockedLessons,
                            const SubjectRequest& request)
{
    auto it = std::find_if(lockedLessons.begin(), lockedLessons.end(), [&](const SubjectWithAddress& subject){
        return subject.SubjectRequestID == request.ID();
    });

    return it != lockedLessons.end();
}

void InitChromosomes(std::vector<std::size_t>& lessons,
                     std::vector<ClassroomAddress>& classrooms,
                     const std::vector<SubjectRequest>& requests,
                     const std::vector<SubjectWithAddress>& lockedLessons,
                     std::size_t requestIndex)
{
    assert(requests.size() == classrooms.size());
    assert(requests.size() == lessons.size());

    const auto& request = requests.at(requestIndex);
    const auto& requestClassrooms = request.Classrooms();

    auto it = std::find_if(lockedLessons.begin(), lockedLessons.end(), [&](const SubjectWithAddress& subject){
        return subject.SubjectRequestID == request.ID();
    });

    if(it != lockedLessons.end())
    {
        lessons.at(requestIndex) = it->Address;
        for(auto&& classroom : requestClassrooms)
        {
            if(!ClassroomsIntersects(lessons, classrooms, it->Address, classroom))
            {
                classrooms.at(requestIndex) = classroom;
                break;
            }
        }

        return;
    }

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
            if(!request.RequestedWeekDay(day))
                continue;

            const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
            if(LessonIsLocked(lockedLessons, scheduleLesson))
                continue;

            if(GroupsOrProfessorsIntersects(requests, lessons, requestIndex, scheduleLesson))
                continue;

            lessons.at(requestIndex) = scheduleLesson;
            if(requestClassrooms.empty())
            {
                classrooms.at(requestIndex) = ClassroomAddress::Any();
                return;
            }

            for(auto&& classroom : requestClassrooms)
            {
                if(!ClassroomsIntersects(lessons, classrooms, scheduleLesson, classroom))
                {
                    classrooms.at(requestIndex) = classroom;
                    return;
                }
            }
        }
    }
}

std::tuple<std::vector<std::size_t>, std::vector<ClassroomAddress>> InitChromosomes(const std::vector<SubjectRequest>& requests,
                                                                                    const std::vector<SubjectWithAddress>& lockedLessons)
{
    std::tuple<std::vector<std::size_t>, std::vector<ClassroomAddress>> chromosomes;
    auto& lessons = std::get<0>(chromosomes);
    auto& classrooms = std::get<1>(chromosomes);

    lessons.resize(requests.size(), NO_LESSON);
    classrooms.resize(requests.size(), ClassroomAddress::NoClassroom());

    for(std::size_t i = 0; i < requests.size(); ++i)
        InitChromosomes(std::get<0>(chromosomes), std::get<1>(chromosomes), requests, lockedLessons, i);

    return chromosomes;
}

bool ReadyToCrossover(const ScheduleIndividual& first,
                      const ScheduleIndividual& second,
                      std::size_t requestIndex)
{
    
    assert(first.Requests() == second.Requests());

    const auto& requests = first.Requests();
    assert(!requests.empty());
    assert(requests.size() == first.Lessons().size());
    assert(requests.size() == second.Lessons().size());
    assert(requests.size() == first.Classrooms().size());
    assert(requests.size() == second.Classrooms().size());

    const auto& thisLessons = first.Lessons();
    const auto& thisClassrooms = first.Classrooms();
    const auto thisLesson = thisLessons.at(requestIndex);
    const auto thisClassroom = thisClassrooms.at(requestIndex);

    const auto& otherLessons = second.Lessons();
    const auto& otherClassrooms = second.Classrooms();
    const auto otherLesson = otherLessons.at(requestIndex);
    const auto otherClassroom = otherClassrooms.at(requestIndex);

    if(ClassroomsIntersects(thisLessons, thisClassrooms, otherLesson, otherClassroom) ||
        ClassroomsIntersects(otherLessons, otherClassrooms, thisLesson, thisClassroom))
        return false;

    if(GroupsOrProfessorsOrClassroomsIntersects(requests, thisLessons, thisClassrooms, requestIndex, otherLesson) ||
        GroupsOrProfessorsOrClassroomsIntersects(requests, otherLessons, otherClassrooms, requestIndex, thisLesson))
        return false;

    return true;
}


std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const std::vector<SubjectRequest>& requests,
                             const std::vector<std::size_t>& lessons,
                             const std::vector<ClassroomAddress>& classrooms)
{
    std::size_t evaluatedValue = 0;

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

    for(std::size_t r = 0; r < lessons.size(); ++r)
    {
        const auto& request = requests.at(r);
        const std::size_t lesson = lessons.at(r);
        if(lesson == NO_LESSON)
        {
            evaluatedValue += 100;
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

            if(classrooms.at(r) == ClassroomAddress::NoClassroom())
            {
                evaluatedValue += 100;
                continue;
            }

            it->second[lessonInDay] = classrooms.at(r).Building;
        }
    }
     
    std::size_t maxComplexityPerDay = 0;
    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
    {
        // evaluating max complexity of day for group
        for(auto&& p : dayComplexity[d])
            maxComplexityPerDay = std::max(maxComplexityPerDay, p.second);

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
                        evaluatedValue += lessonsGap * 3;
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
                        evaluatedValue += lessonsGap * 2;
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
                    evaluatedValue += 64;

                prevBuilding = currentBuilding;
            }
        }
    }

    evaluatedValue += maxComplexityPerDay;
    return evaluatedValue;
}


ScheduleIndividual::ScheduleIndividual(std::random_device& randomDevice,
                                       const std::vector<SubjectRequest>* pRequests,
                                       const std::vector<SubjectWithAddress>* pLocked)
    : pRequests_(pRequests)
    , pLocked_(pLocked)
    , evaluatedValue_(NOT_EVALUATED)
    , classrooms_()
    , lessons_()
    , buffer_(DEFAULT_BUFFER_SIZE)
    , randomGenerator_(randomDevice())
{
    assert(pRequests_ != nullptr);
    assert(pLocked_ != nullptr);
    assert(std::is_sorted(pLocked_->begin(), pLocked_->end(), SubjectWithAddressLess()));

    std::tie(lessons_, classrooms_) = InitChromosomes(Requests(), *pLocked_);
}

void ScheduleIndividual::swap(ScheduleIndividual& other) noexcept
{
    std::swap(evaluatedValue_, other.evaluatedValue_);
    std::swap(classrooms_, other.classrooms_);
    std::swap(lessons_, other.lessons_);
}

ScheduleIndividual::ScheduleIndividual(const ScheduleIndividual& other)
    : pRequests_(other.pRequests_)
    , pLocked_(other.pLocked_)
    , evaluatedValue_(other.evaluatedValue_)
    , classrooms_(other.classrooms_)
    , lessons_(other.lessons_)
    , buffer_(other.buffer_.size())
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(const ScheduleIndividual& other)
{
    ScheduleIndividual tmp(other);
    tmp.swap(*this);
    return *this;
}

ScheduleIndividual::ScheduleIndividual(ScheduleIndividual&& other) noexcept
    : pRequests_(other.pRequests_)
    , pLocked_(other.pLocked_)
    , evaluatedValue_(other.evaluatedValue_)
    , classrooms_(std::move(other.classrooms_))
    , lessons_(std::move(other.lessons_))
    , buffer_(other.buffer_.size())
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(ScheduleIndividual&& other) noexcept
{
    other.swap(*this);
    return *this;
}

const std::vector<SubjectRequest>& ScheduleIndividual::Requests() const { return *pRequests_; }
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

    std::uniform_int_distribution<std::size_t> requestsDistrib(0, pRequests_->size() - 1);
    const std::size_t requestIndex = requestsDistrib(randomGenerator_);

    std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
    if(headsOrTails(randomGenerator_))
        ChangeClassroom(requestIndex);
    else
        ChangeLesson(requestIndex);
}

std::size_t ScheduleIndividual::Evaluate() const
{
    if(evaluatedValue_ != NOT_EVALUATED)
        return evaluatedValue_;

    LinearAllocatorBufferSpan bufferSpan(buffer_.data(), buffer_.size());
    
    evaluatedValue_ = EvaluateSchedule(bufferSpan, *pRequests_, lessons_, classrooms_);

    buffer_.resize(std::max(bufferSpan.peak, buffer_.size()));
    return evaluatedValue_;
}

void ScheduleIndividual::Crossover(ScheduleIndividual& other)
{
    std::uniform_int_distribution<std::size_t> requestsDist(0, pRequests_->size() - 1);
    const auto requestIndex = requestsDist(randomGenerator_);
    if(ReadyToCrossover(*this, other, requestIndex))
    {
        evaluatedValue_ = NOT_EVALUATED;
        other.evaluatedValue_ = NOT_EVALUATED;

        // crossovering
        std::swap(classrooms_.at(requestIndex), other.classrooms_.at(requestIndex));
        std::swap(lessons_.at(requestIndex), other.lessons_.at(requestIndex));
    }
}


void ScheduleIndividual::ChangeClassroom(std::size_t requestIndex)
{
    const auto& request = pRequests_->at(requestIndex);
    const auto& classrooms = request.Classrooms();

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() && ClassroomsIntersects(lessons_, classrooms_, lessons_.at(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));
        ++chooseClassroomTry;
    }

    if(chooseClassroomTry < classrooms.size())
    {
        classrooms_.at(requestIndex) = scheduleClassroom;
        evaluatedValue_ = NOT_EVALUATED;
    }
}

void ScheduleIndividual::ChangeLesson(std::size_t requestIndex)
{
    const auto& request = pRequests_->at(requestIndex);
    if(RequestHasLockedLesson(*pLocked_, request))
        return;

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
    std::size_t scheduleLesson = lessonsDistrib(randomGenerator_);

    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT && 
        (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
            GroupsOrProfessorsOrClassroomsIntersects(*pRequests_, lessons_, classrooms_, requestIndex, scheduleLesson)))
    {
        scheduleLesson = lessonsDistrib(randomGenerator_);
        ++chooseLessonTry;
    }

    if(chooseLessonTry < MAX_LESSONS_COUNT)
    {
        lessons_.at(requestIndex) = scheduleLesson;
        evaluatedValue_ = NOT_EVALUATED;
    }
}


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs) { lhs.swap(rhs); }

void Print(const ScheduleIndividual& individ)
{
    const auto& requests = individ.Requests();
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
                std::cout << "[s:" << request.ID() <<
                    ", p:" << request.Professor() <<
                    ", c:(" << classrooms.at(r).Building << ", " << classrooms.at(r).Classroom << "), g: {";

                for(auto&& g : request.Groups())
                    std::cout << ' ' << g;

                std::cout << " }]";

                it = std::find(std::next(it), lessons.end(), l);
            }
        }

        std::cout << '\n';
    }

    std::cout.flush();
}

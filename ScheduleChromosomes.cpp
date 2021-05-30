#include "ScheduleChromosomes.h"
#include "utils.h"


static constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();
static constexpr std::size_t NOT_EVALUATED = std::numeric_limits<std::size_t>::max();
static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024;


ScheduleChromosomes::ScheduleChromosomes(std::vector<std::size_t> lessons,
                                         std::vector<ClassroomAddress> classrooms)
    : lessons_(std::move(lessons))
    , classrooms_(std::move(classrooms))
{
    assert(lessons_.size() == classrooms_.size());
}

ScheduleChromosomes::ScheduleChromosomes(const ScheduleData& data)
    : lessons_(data.SubjectRequests().size(), NO_LESSON)
    , classrooms_(data.SubjectRequests().size(), ClassroomAddress::NoClassroom())
{
    assert(!data.SubjectRequests().empty());
    for(auto&& locked : data.LockedLessons())
    {
        const std::size_t r = data.IndexOfSubjectRequestWithID(locked.SubjectRequestID);
        lessons_.at(r) = locked.Address;

        auto&& request = data.SubjectRequests().at(r);
        for(auto&& classroom : request.Classrooms())
        {
            if(!ClassroomsIntersects(locked.Address, classroom))
            {
                classrooms_.at(r) = classroom;
                break;
            }
        }
    }

    for(std::size_t r = 0; r < data.SubjectRequests().size(); ++r)
        InitFromRequest(data, r);
}

void ScheduleChromosomes::InitFromRequest(const ScheduleData& data, 
                                          std::size_t requestIndex)
{
    const auto& requests = data.SubjectRequests();
    const auto& request = requests.at(requestIndex);
    const auto& requestClassrooms = request.Classrooms();
    const auto& lockedLessons = data.LockedLessons();

    if(data.SubjectRequestHasLockedLesson(request))
        return;

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
            if(!request.RequestedWeekDay(day))
                continue;

            const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
            if(IsLateScheduleLessonInSaturday(scheduleLesson))
                continue;

            if(GroupsOrProfessorsIntersects(data, requestIndex, scheduleLesson))
                continue;

            lessons_.at(requestIndex) = scheduleLesson;
            if(requestClassrooms.empty())
            {
                classrooms_.at(requestIndex) = ClassroomAddress::Any();
                return;
            }

            for(auto&& classroom : requestClassrooms)
            {
                if(!ClassroomsIntersects(scheduleLesson, classroom))
                {
                    classrooms_.at(requestIndex) = classroom;
                    return;
                }
            }
        }
    }
}

bool ScheduleChromosomes::GroupsOrProfessorsOrClassroomsIntersects(const ScheduleData& data, 
                                                                   std::size_t currentRequest,
                                                                   std::size_t currentLesson) const
{
    if(classrooms_.at(currentRequest) == ClassroomAddress::Any())
        return GroupsOrProfessorsIntersects(data, currentRequest, currentLesson);

    const auto& requests = data.SubjectRequests();
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons_.begin(), lessons_.end(), currentLesson);
    while(it != lessons_.end())
    {
        const std::size_t requestIndex = std::distance(lessons_.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || 
           classrooms_.at(currentRequest) == classrooms_.at(requestIndex) || 
           set_intersects(thisRequest.Groups(), otherRequest.Groups()))
        {
            return true;
        }

        it = std::find(std::next(it), lessons_.end(), currentLesson);
    }

    return false;
}

bool ScheduleChromosomes::GroupsOrProfessorsIntersects(const ScheduleData& data,
                                                       std::size_t currentRequest,
                                                       std::size_t currentLesson) const
{
    const auto& requests = data.SubjectRequests();
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons_.begin(), lessons_.end(), currentLesson);
    while(it != lessons_.end())
    {
        const std::size_t requestIndex = std::distance(lessons_.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || set_intersects(thisRequest.Groups(), otherRequest.Groups()))
            return true;

        it = std::find(std::next(it), lessons_.end(), currentLesson);
    }

    return false;
}

bool ScheduleChromosomes::ClassroomsIntersects(std::size_t currentLesson,
                                               const ClassroomAddress& currentClassroom) const
{
    if(currentClassroom == ClassroomAddress::Any())
        return false;

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

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r)
{
    const auto firstLesson = first.Lesson(r);
    const auto firstClassroom = first.Classroom(r);

    const auto secondLesson = second.Lesson(r);
    const auto secondClassroom = second.Classroom(r);

    if(first.ClassroomsIntersects(secondLesson, secondClassroom) ||
        second.ClassroomsIntersects(firstLesson, firstClassroom))
        return false;

    if(first.GroupsOrProfessorsOrClassroomsIntersects(data, r, secondLesson) ||
        second.GroupsOrProfessorsOrClassroomsIntersects(data, r, firstLesson))
        return false;

    return true;
}

void Crossover(ScheduleChromosomes& first,
               ScheduleChromosomes& second,
               std::size_t r)
{
    using std::swap;
    swap(first.Lesson(r), second.Lesson(r));
    swap(first.Classroom(r), second.Classroom(r));
}


std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const ScheduleData& scheduleData,
                             const ScheduleChromosomes& scheduleChromosomes)
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

    const auto requests = scheduleData.SubjectRequests();
    for(std::size_t r = 0; r < requests.size(); ++r)
    {
        const auto& request = requests.at(r);
        const std::size_t lesson = scheduleChromosomes.Lesson(r);
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

            if(scheduleChromosomes.Classroom(r) == ClassroomAddress::NoClassroom())
            {
                evaluatedValue += 100;
                continue;
            }

            it->second[lessonInDay] = scheduleChromosomes.Classroom(r).Building;
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

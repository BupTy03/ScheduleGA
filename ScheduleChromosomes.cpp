#include "ScheduleChromosomes.h"
#include "utils.h"

#include <range/v3/all.hpp>


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

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes,
                     const ScheduleData& scheduleData)
{
    std::size_t maxBuildingsDayEval = 0;
    std::size_t maxDayComplexity = 0;
    std::size_t maxLessonsGapsForGroupsSum = 0;
    std::size_t maxLessonsGapsForProfessorsSum = 0;

    const auto& requests = scheduleData.SubjectRequests();

    for(auto&&[professor, professorRequests] : scheduleData.Professors())
    {
        auto professorLessons = professorRequests | ranges::view::transform([&](std::size_t r){ return scheduleChromosomes.Lesson(r); });
        for(std::size_t day : ranges::view::iota(0, DAYS_IN_SCHEDULE))
        {
            const auto firstLesson = day * MAX_LESSONS_PER_DAY;
		    const auto lastLesson = firstLesson + MAX_LESSONS_PER_DAY;

            std::size_t dayLessonsGapsSum = 0;
            std::size_t prevLesson = 0;
		    for(std::size_t lesson : professorLessons | ranges::view::filter([&](std::size_t l){ return l >= firstLesson && l <= lastLesson; }))
		    {
                const auto lessonInDay = lesson % MAX_LESSONS_PER_DAY;
                const auto lessonsGap = lessonInDay - prevLesson;
			    if(lessonsGap > 1)
                    dayLessonsGapsSum += (lessonsGap - 1);
                
                prevLesson = lessonInDay;
            }

            maxLessonsGapsForProfessorsSum = std::max(maxLessonsGapsForProfessorsSum, dayLessonsGapsSum);
        }
    }

    for(auto&&[group, groupRequests] : scheduleData.Groups())
    {
        auto groupLessons = ranges::view::zip(groupRequests | ranges::view::transform([&](std::size_t r){ return scheduleChromosomes.Lesson(r); }), groupRequests);
        for(std::size_t day : ranges::view::iota(0, DAYS_IN_SCHEDULE))
        {
            const auto firstLesson = day * MAX_LESSONS_PER_DAY;
		    const auto lastLesson = firstLesson + MAX_LESSONS_PER_DAY;

            std::size_t dayLessonsGapsSum = 0;
			std::size_t dayBuldingsChange = 0;
            std::size_t dayComplexity = 0;

			std::size_t prevBuilding = NO_BUILDING;
			std::size_t prevLesson = 0;
			for(auto&&[lesson, index] : groupLessons | ranges::view::filter([&](auto&& p){ return p.first >= firstLesson && p.first <= lastLesson; }))
			{
				const auto& classroom = scheduleChromosomes.Classroom(index);
                const auto lessonInDay = lesson % MAX_LESSONS_PER_DAY;
                const auto lessonsGap = lessonInDay - prevLesson;
				if(!(prevBuilding == classroom.Building || classroom.Building == NO_BUILDING || prevBuilding == NO_BUILDING || lessonsGap > 1))
					dayBuldingsChange += 1;

                if(lessonsGap > 1)
                    dayLessonsGapsSum += (lessonsGap - 1);

                dayComplexity += lessonInDay * scheduleData.SubjectRequests().at(index).Complexity();

				prevLesson = lessonInDay;
				prevBuilding = classroom.Building;
			}

            maxBuildingsDayEval = std::max(maxBuildingsDayEval, dayBuldingsChange);
            maxDayComplexity = std::max(maxDayComplexity, dayComplexity);
            maxLessonsGapsForGroupsSum = std::max(maxLessonsGapsForGroupsSum, dayLessonsGapsSum);
        }
    }

    return maxLessonsGapsForGroupsSum * 3 + maxLessonsGapsForProfessorsSum * 2 + maxDayComplexity * 4 + maxBuildingsDayEval * 64;
}

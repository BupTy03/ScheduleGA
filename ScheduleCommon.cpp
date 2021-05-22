#include "ScheduleCommon.h"

#include <cassert>
#include <algorithm>


SubjectRequest::SubjectRequest(std::size_t professor,
                               std::size_t complexity,
                               std::vector<bool> weekDays,
                               std::vector<std::size_t> groups,
                               std::vector<ClassroomAddress> classrooms)
    : professor_(professor)
    , complexity_(complexity)
    , weekDays_(std::move(weekDays))
    , groups_(std::move(groups))
    , classrooms_(std::move(classrooms))
{
    assert(weekDays_.size() == DAYS_IN_SCHEDULE);
    if(std::all_of(weekDays_.begin(), weekDays_.end(), [](bool wd){return wd;}))
        weekDays_.assign(weekDays.size(), true);

    std::sort(groups_.begin(), groups_.end());
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::sort(classrooms_.begin(), classrooms_.end());
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());
}

bool SubjectRequest::RequestedClassroom(const ClassroomAddress& classroomAddress) const
{
    auto it = std::lower_bound(classrooms_.begin(), classrooms_.end(), classroomAddress);
    return it != classrooms_.end() && *it == classroomAddress;
}

bool SubjectRequest::RequestedGroup(std::size_t g) const
{
    auto it = std::lower_bound(groups_.begin(), groups_.end(), g);
    return it != groups_.end() && *it == g;
}

bool SubjectRequest::RequestedWeekDay(std::size_t d) const { return weekDays_.at(d); }
const std::vector<std::size_t>& SubjectRequest::Groups() const { return groups_; }
const std::vector<ClassroomAddress>& SubjectRequest::Classrooms() const { return classrooms_; }
std::size_t SubjectRequest::Complexity() const { return complexity_; }
std::size_t SubjectRequest::Professor() const { return professor_; }

#include "ScheduleCommon.h"

#include <string>
#include <cassert>
#include <stdexcept>
#include <algorithm>


SubjectRequest::SubjectRequest(std::size_t id,
                               std::size_t professor,
                               std::size_t complexity,
                               std::vector<bool> weekDays,
                               std::vector<std::size_t> groups,
                               std::vector<ClassroomAddress> classrooms)
    : id_(id)
    , professor_(professor)
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
std::size_t SubjectRequest::ID() const { return id_; }
std::size_t SubjectRequest::Complexity() const { return complexity_; }
std::size_t SubjectRequest::Professor() const { return professor_; }


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> lockedLessons)
        : subjectRequests_(std::move(subjectRequests))
        , lockedLessonsSortedBySubjectID_(lockedLessons)
        , lockedLessonsSortedByAddress_(std::move(lockedLessons))
{
    std::sort(lockedLessonsSortedBySubjectID_.begin(), lockedLessonsSortedBySubjectID_.end(), SubjectWithAddressLessBySubjectID());
    std::sort(lockedLessonsSortedByAddress_.begin(), lockedLessonsSortedByAddress_.end(), SubjectWithAddressLessByAddress());

    std::sort(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDLess());
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDEqual()), subjectRequests_.end());
}

const std::vector<SubjectRequest>& ScheduleData::SubjectRequests() const { return subjectRequests_; }

bool ScheduleData::LessonIsLocked(std::size_t lessonAddress) const
{
    return std::binary_search(lockedLessonsSortedByAddress_.begin(), 
                              lockedLessonsSortedByAddress_.end(), 
                              lessonAddress, SubjectWithAddressLessByAddress());
}

bool ScheduleData::RequestHasLockedLesson(const SubjectRequest& request) const
{
    return std::binary_search(lockedLessonsSortedBySubjectID_.begin(), 
                              lockedLessonsSortedBySubjectID_.end(), 
                              request.ID(), SubjectWithAddressLessBySubjectID());
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::lower_bound(subjectRequests_.begin(), subjectRequests_.end(), subjectRequestID, SubjectRequestIDLess());
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

const std::vector<SubjectWithAddress>& ScheduleData::LockedLessons() const
{
    return lockedLessonsSortedByAddress_;
}

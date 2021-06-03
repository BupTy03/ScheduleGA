#include "ScheduleCommon.h"

#include <string>
#include <stdexcept>


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
    if(weekDays_.empty())
        weekDays_.assign(weekDays.size(), true);

    std::ranges::sort(groups_);
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::ranges::sort(classrooms_);
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());
}

bool SubjectRequest::RequestedClassroom(const ClassroomAddress& classroomAddress) const
{
    auto it = std::ranges::lower_bound(classrooms_, classroomAddress);
    return it != classrooms_.end() && *it == classroomAddress;
}

bool SubjectRequest::RequestedGroup(std::size_t g) const
{
    auto it = std::ranges::lower_bound(groups_, g);
    return it != groups_.end() && *it == g;
}

bool SubjectRequest::RequestedWeekDay(std::size_t d) const { return weekDays_.at(d % DAYS_IN_SCHEDULE_WEEK); }
const std::vector<std::size_t>& SubjectRequest::Groups() const { return groups_; }
const std::vector<ClassroomAddress>& SubjectRequest::Classrooms() const { return classrooms_; }
std::size_t SubjectRequest::ID() const { return id_; }
std::size_t SubjectRequest::Complexity() const { return complexity_; }
std::size_t SubjectRequest::Professor() const { return professor_; }


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> lockedLessons)
    : subjectRequests_(std::move(subjectRequests))
    , lockedLessons_(std::move(lockedLessons))
    , professorRequests_()
    , groupRequests_()
{
    std::ranges::sort(subjectRequests_, {}, &SubjectRequest::ID);
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(),
                                       SubjectRequestIDEqual()), subjectRequests_.end());

    std::ranges::sort(lockedLessons_, {}, &SubjectWithAddress::SubjectRequestID);
    lockedLessons_.erase(std::unique(lockedLessons_.begin(), lockedLessons_.end()), lockedLessons_.end());

    for(std::size_t r = 0; r < subjectRequests_.size(); ++r)
    {
        const auto& request = subjectRequests_.at(r);
        professorRequests_[request.Professor()].insert(r);
        for(std::size_t g : request.Groups())
            groupRequests_[g].insert(r);
    }
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

std::size_t ScheduleData::IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return std::distance(subjectRequests_.begin(), it);
}

bool ScheduleData::SubjectRequestHasLockedLesson(const SubjectRequest& request) const
{
    return std::ranges::binary_search(lockedLessons_, request.ID(), {}, &SubjectWithAddress::SubjectRequestID);
}

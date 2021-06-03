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


ScheduleDataGenerator::ScheduleDataGenerator(std::random_device& randDevice,
                                             std::size_t minGroupsCount,
                                             std::size_t maxGroupsCount,
                                             std::size_t minClassroomsCount,
                                             std::size_t maxClassroomsCount)
    : randGen_(randDevice())
    , minGroupsCount_(minGroupsCount)
    , maxGroupsCount_(maxGroupsCount)
    , minClassroomsCount_(minClassroomsCount)
    , maxClassroomsCount_(maxClassroomsCount)
{
}

std::vector<bool> ScheduleDataGenerator::GenerateRandomWeekDays()
{
	std::uniform_int_distribution<int> dist(0, 1);
	std::vector<bool> weekDays;
	for(std::size_t i = 0; i < DAYS_IN_SCHEDULE_WEEK; ++i)
		weekDays.emplace_back(dist(randGen_));
	
	return weekDays;
}

std::size_t ScheduleDataGenerator::GenerateRandomVal(std::size_t minID, std::size_t maxID)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<std::size_t> dist(minID, maxID);
	return dist(gen);
}

std::size_t ScheduleDataGenerator::GenerateRandomID(std::size_t maxID)
{
	return GenerateRandomVal(0, maxID);
}

std::vector<std::size_t> ScheduleDataGenerator::GenerateIDArray(std::size_t n)
{
	std::vector<std::size_t> result(n);
	for(auto& v : result)
		v = GenerateRandomID();

	return result;
}

std::vector<ClassroomAddress> ScheduleDataGenerator::GenerateRandomClassrooms(std::size_t n)
{
	std::vector<ClassroomAddress> result(n);
	for(auto& c : result)
		c = ClassroomAddress(GenerateRandomID(5), GenerateRandomID(1000));

	return result;
}

std::vector<SubjectRequest> ScheduleDataGenerator::GenerateSubjectRequests(std::size_t n)
{
    std::vector<SubjectRequest> requests;
    requests.reserve(n);
	for(std::size_t i = 0; i < n; ++i)
	{
		requests.emplace_back(i, 
                              GenerateRandomID(),
                              GenerateRandomVal(MIN_COMPLEXITY, MAX_COMPLEXITY),
                              GenerateRandomWeekDays(),
                              GenerateIDArray(GenerateRandomVal(minGroupsCount_, maxGroupsCount_)),
                              GenerateRandomClassrooms(GenerateRandomVal(minClassroomsCount_, maxClassroomsCount_)));
	}
    return requests;
}

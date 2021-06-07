#pragma once
#include "utils.h"

#include <array>
#include <random>
#include <limits>
#include <vector>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>


constexpr auto MAX_LESSONS_PER_DAY = 7;
constexpr auto DAYS_IN_SCHEDULE_WEEK = 6;
constexpr auto DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr auto MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE_WEEK * 2;
constexpr auto RECOMMENDED_LESSONS_COUNT = 3;
constexpr auto NO_BUILDING = std::numeric_limits<std::size_t>::max();
constexpr auto NO_LESSON = std::numeric_limits<std::size_t>::max();
constexpr auto MIN_COMPLEXITY = 1;
constexpr auto MAX_COMPLEXITY = 4;


struct ScheduleItem
{
   explicit ScheduleItem(std::size_t professor,
                         std::size_t group,
                         std::size_t classroom)
      : Professor(professor)
      , Group(group)
      , Classroom(classroom)
   {
   }

   std::size_t Professor;
   std::size_t Group;
   std::size_t Classroom;
};


struct ClassroomAddress
{
    ClassroomAddress() = default;
    ClassroomAddress(std::size_t building, std::size_t classroom)
        : Building(building)
        , Classroom(classroom)
    {}

    static ClassroomAddress Any() { return ClassroomAddress(0, 0); }
    static ClassroomAddress NoClassroom()
    {
       return ClassroomAddress(std::numeric_limits<std::size_t>::max(),
                            std::numeric_limits<std::size_t>::max());
    }

    friend bool operator==(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return lhs.Building == rhs.Building && lhs.Classroom == rhs.Classroom;
    }
    friend bool operator!=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs == rhs); }

    friend bool operator<(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return (lhs.Building < rhs.Building) || (lhs.Building == rhs.Building && lhs.Classroom < rhs.Classroom);
    }
    friend bool operator>(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return rhs < lhs; }
    friend bool operator<=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(rhs < lhs); }
    friend bool operator>=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs < rhs); }

    std::size_t Building;
    std::size_t Classroom;
};


class SubjectRequest
{
public:
   explicit SubjectRequest(std::size_t id,
                           std::size_t professor,
                           std::size_t complexity,
                           std::vector<bool> weekDays,
                           std::vector<std::size_t> groups,
                           std::vector<ClassroomAddress> classrooms);

   bool RequestedClassroom(const ClassroomAddress& classroomAddress) const;
   bool RequestedGroup(std::size_t g) const;
   bool RequestedWeekDay(std::size_t d) const;

   const std::vector<std::size_t>& Groups() const;
   const std::vector<ClassroomAddress>& Classrooms() const;

   std::size_t ID() const;
   std::size_t Complexity() const;
   std::size_t Professor() const;

   friend bool operator==(const SubjectRequest& lhs, const SubjectRequest& rhs)
   {
       return lhs.professor_ == rhs.professor_ &&
       lhs.complexity_ == rhs.complexity_ &&
       lhs.weekDays_ == rhs.weekDays_ &&
       lhs.groups_ == rhs.groups_ &&
       lhs.classrooms_ == rhs.classrooms_;
   }
   friend bool operator!=(const SubjectRequest& lhs, const SubjectRequest& rhs)
   {
       return !(lhs == rhs);
   }

private:
    std::size_t id_;
    std::size_t professor_;
    std::size_t complexity_;
    std::vector<bool> weekDays_;
    std::vector<std::size_t> groups_;
    std::vector<ClassroomAddress> classrooms_;
};


struct SubjectRequestIDLess
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() < rhs.ID();
    }
    bool operator()(const SubjectRequest& lhs, std::size_t rhsID) const
    {
        return lhs.ID() < rhsID;
    }
    bool operator()(std::size_t lhsID, const SubjectRequest& rhs) const
    {
        return lhsID < rhs.ID();
    }
};

struct SubjectRequestIDEqual
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() == rhs.ID();
    }
};


struct SubjectWithAddress
{
    SubjectWithAddress() = default;
    explicit SubjectWithAddress(std::size_t SubjectRequestID, std::size_t Address)
        : SubjectRequestID(SubjectRequestID)
        , Address(Address)
    {}

    friend bool operator==(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.SubjectRequestID == rhs.SubjectRequestID && lhs.Address == rhs.Address;
    }

    friend bool operator!=(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t SubjectRequestID = 0;
    std::size_t Address = 0;
};

struct SubjectWithAddressLessByAddress
{
    bool operator()(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs) const
    {
        return lhs.Address < rhs.Address;
    }

    bool operator()(const SubjectWithAddress& lhs, std::size_t rhsAddress) const
    {
        return lhs.Address < rhsAddress;
    }

    bool operator()(std::size_t lhsAddress, const SubjectWithAddress& rhs) const
    {
        return lhsAddress < rhs.Address;
    }
};

struct SubjectWithAddressLessBySubjectID
{
    bool operator()(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs) const
    {
        return lhs.SubjectRequestID < rhs.SubjectRequestID;
    }

    bool operator()(const SubjectWithAddress& lhs, std::size_t rhsSubjectRequestID) const
    {
        return lhs.SubjectRequestID < rhsSubjectRequestID;
    }

    bool operator()(std::size_t lhsrhsSubjectRequestID, const SubjectWithAddress& rhs) const
    {
        return lhsrhsSubjectRequestID < rhs.SubjectRequestID;
    }
};


class ScheduleData
{
public:
    ScheduleData() = default;
    explicit ScheduleData(std::vector<SubjectRequest> subjectRequests,
                          std::vector<SubjectWithAddress> lockedLessons = {});

    const std::vector<SubjectRequest>& SubjectRequests() const { return subjectRequests_; }
    const SubjectRequest& SubjectRequestAtID(std::size_t subjectRequestID) const;
    std::size_t IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const;

    const std::vector<SubjectWithAddress>& LockedLessons() const { return lockedLessons_; }
    bool SubjectRequestHasLockedLesson(const SubjectRequest& request) const;

    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Professors() const { return professorRequests_; }
    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Groups() const { return groupRequests_; }

private:
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<SubjectWithAddress> lockedLessons_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> professorRequests_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> groupRequests_;
};


class ScheduleDataGenerator
{
public:
    explicit ScheduleDataGenerator(std::random_device& randDevice,
                                   std::size_t minGroupsCount,
                                   std::size_t maxGroupsCount,
                                   std::size_t minClassroomsCount,
                                   std::size_t maxClassroomsCount);

    std::vector<bool> GenerateRandomWeekDays();
    std::size_t GenerateRandomVal(std::size_t minID, std::size_t maxID);
    std::size_t GenerateRandomID(std::size_t maxID = 1000);
    std::vector<std::size_t> GenerateIDArray(std::size_t n);
    std::vector<ClassroomAddress> GenerateRandomClassrooms(std::size_t n);
    std::vector<SubjectRequest> GenerateSubjectRequests(std::size_t n);

private:
    std::size_t minGroupsCount_;
    std::size_t maxGroupsCount_;
    std::size_t minClassroomsCount_;
    std::size_t maxClassroomsCount_;
    std::mt19937 randGen_;
};

template<typename T>
std::vector<T> Merge(const std::vector<T>& lhs,
                     const std::vector<T>& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    std::vector<T> tmp;
    std::merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::back_inserter(tmp));
    return tmp;
}

constexpr bool IsLateScheduleLessonInSaturday(std::size_t l)
{
    constexpr std::array lateSaturdayLessonsTable = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,
    };

    static_assert(lateSaturdayLessonsTable.size() == MAX_LESSONS_COUNT, "re-fill lateSaturdayLessonsTable");
    return lateSaturdayLessonsTable[l];
}

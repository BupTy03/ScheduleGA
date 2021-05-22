#pragma once
#include <limits>
#include <vector>


constexpr auto MAX_LESSONS_PER_DAY = 7;
constexpr auto DAYS_IN_SCHEDULE_WEEK = 6;
constexpr auto DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr auto MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE_WEEK * 2;
constexpr auto RECOMMENDED_LESSONS_COUNT = 3;
constexpr auto NO_BUILDING = std::numeric_limits<std::size_t>::max();


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
    explicit ClassroomAddress(std::size_t building, std::size_t classroom)
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
   explicit SubjectRequest(std::size_t professor,
                           std::size_t complexity,
                           std::vector<bool> weekDays,
                           std::vector<std::size_t> groups,
                           std::vector<ClassroomAddress> classrooms);

   bool RequestedClassroom(const ClassroomAddress& classroomAddress) const;
   bool RequestedGroup(std::size_t g) const;
   bool RequestedWeekDay(std::size_t d) const;

   const std::vector<std::size_t>& Groups() const;
   const std::vector<ClassroomAddress>& Classrooms() const;

   std::size_t Complexity() const;
   std::size_t Professor() const;

private:
    std::size_t professor_;
    std::size_t complexity_;
    std::vector<bool> weekDays_;
    std::vector<std::size_t> groups_;
    std::vector<ClassroomAddress> classrooms_;
};

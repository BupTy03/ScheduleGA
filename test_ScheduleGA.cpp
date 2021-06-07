#include <catch2/catch.hpp>

#include "ScheduleCommon.h"
#include "ScheduleIndividual.h"


TEST_CASE("Initialization of chromosomes is valid", "[ScheduleChromosomes]")
{
    const std::vector<bool> fullWeek(6, true);
    const std::vector<bool> weekDays{true, false, true, true, false, false};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest{0, 1, 1, fullWeek, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{1, 2, 1, weekDays, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{2, 1, 1, fullWeek, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{3, 4, 1, fullWeek, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}},
        SubjectRequest{4, 5, 1, fullWeek, {10},      {{0, 1}, {0, 2}, {0, 3}}}
    };

    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{data};

    SECTION("No ovelaps by professors")
    {
        REQUIRE(sut.Lesson(0) != sut.Lesson(2));
    }
    SECTION("No overlaps by groups")
    {
        REQUIRE(sut.Lesson(0) != sut.Lesson(1));
    }
    SECTION("No overlaps by classrooms")
    {
        for(std::size_t i = 0; i < requests.size(); ++i)
        {
            for(std::size_t j = 0; j < requests.size(); ++j)
            {
                if(i == j)
                    continue;

                REQUIRE((sut.Lesson(i) != sut.Lesson(j) || sut.Classroom(i) != sut.Classroom(j)));
            }
        }
    }
}

TEST_CASE("Check if groups or professors or classrooms intersects", "[ScheduleChromosomes]")
{
    const std::vector weekDays{true, true, true, true, true, true};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, weekDays, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(1, 2, 1, weekDays, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(2, 1, 1, weekDays, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(3, 4, 1, weekDays, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(4, 5, 1, weekDays, {10},      {{0, 1}, {0, 2}, {0, 3}})
    };
    const ScheduleData data{requests, {}};
    const ScheduleChromosomes sut{{0, 1, 2, 3, 4},
                                  {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    SECTION("Check if groups intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 1, 0));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 1, 0));
    }
    SECTION("Check if professors intersects")
    {
        REQUIRE(sut.GroupsOrProfessorsIntersects(data, 2, 2));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 2, 2));
    }
    SECTION("Check if classrooms intersects")
    {
        REQUIRE(sut.ClassroomsIntersects(0, {0, 1}));
        REQUIRE(sut.GroupsOrProfessorsOrClassroomsIntersects(data, 3, 0));
    }
}

TEST_CASE("Check if chromosomes ready crossover", "[ScheduleChromosomes]")
{
    const std::vector weekDays{true, true, true, true, true, true};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, weekDays, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(1, 2, 1, weekDays, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(2, 1, 1, weekDays, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(3, 4, 1, weekDays, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(4, 5, 1, weekDays, {10},      {{0, 1}, {0, 2}, {0, 3}})
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes sut1{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes sut2{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,2}, {0,2}}};

    REQUIRE(ReadyToCrossover(sut1, sut2, data, 0));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 1));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 2));
    REQUIRE_FALSE(ReadyToCrossover(sut1, sut2, data, 3));
    REQUIRE(ReadyToCrossover(sut1, sut2, data, 4));
}

TEST_CASE("Crossover works", "[ScheduleChromosomes]")
{
    const std::vector weekDays{true, true, true, true, true, true};
    const std::vector requests {
        // [id, professor, complexity, weekDays, groups, classrooms]
        SubjectRequest(0, 1, 1, weekDays, {0, 1, 2}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(1, 2, 1, weekDays, {1, 2, 3}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(2, 1, 1, weekDays, {4, 5, 6}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(3, 4, 1, weekDays, {7, 8, 9}, {{0, 1}, {0, 2}, {0, 3}}),
        SubjectRequest(4, 5, 1, weekDays, {10},      {{0, 1}, {0, 2}, {0, 3}})
    };
    const ScheduleData data{requests, {}};

    ScheduleChromosomes first{{0, 1, 2, 3, 4}, {{0,3}, {0,2}, {0,1}, {0,3}, {0,2}}};
    ScheduleChromosomes second{{4, 3, 2, 1, 0}, {{0,1}, {0,2}, {0,3}, {0,1}, {0,2}}};

    Crossover(first, second, 0);
    REQUIRE(first.Lesson(0) == 4);
    REQUIRE(first.Classroom(0) == ClassroomAddress{0,1});

    REQUIRE(second.Lesson(0) == 0);
    REQUIRE(second.Classroom(0) == ClassroomAddress{0,3});
}

TEST_CASE("Counting unassigned lessons correctly")
{
    ScheduleChromosomes sut{{0, NO_LESSON, 2, 3, NO_LESSON, NO_LESSON, 6, NO_LESSON}, 
                            {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}};

    REQUIRE(sut.UnassignedLessonsCount() == 4);
}

TEST_CASE("Counting unassigned classrooms correctly")
{
    ScheduleChromosomes sut{
        {0, 1, 2, 3, 4, 5, 6, 7},
        {{0,0}, ClassroomAddress::NoClassroom(), {0,0}, {0,0}, 
        ClassroomAddress::NoClassroom(), ClassroomAddress::NoClassroom(), {0,0}, {0,0}}
    };

    REQUIRE(sut.UnassignedClassroomsCount() == 3);
}

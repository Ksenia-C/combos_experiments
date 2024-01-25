#pragma once
#include <vector>
#include <string>
#include <format>
#include <algorithm>
#include <mutex>
#include <map>
#include <set>
#include <fstream>
#include <simgrid/s4u.hpp>
#include "components/types.hpp"
#include "components/shared.hpp"

enum class PassedPeriod
{
    CreatedAt,
    ChosenToSend,
    MovedToClient,
    MovedToRunable,
    Ran,
    Stopped,
    Wait,
    BeforeMissedDeadline,
    BeforeSendBack,
    BeforeValidation,
};

void push_new_stat(int result_id, std::string workunit_id, PassedPeriod event_type);

void post_stats_work();
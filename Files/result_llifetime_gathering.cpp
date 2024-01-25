#include "result_llifetime_gathering.hpp"
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

struct ResultTimeline
{
    int result_id;
    std::string workunit_id;
    double last_tmp;
    std::vector<std::pair<PassedPeriod, double>> events;
};

// not cool but it's quicker
struct LifetimeT
{
    std::map<std::string, ResultTimeline> results;
    std::mutex mutex_;
};

static LifetimeT all_result_liftetime;

void push_new_stat(int result_id, std::string workunit_id, PassedPeriod event_type)
{
    double timestamp = sg4::Engine::get_clock();
    std::unique_lock lock(all_result_liftetime.mutex_);
    auto &timeline = all_result_liftetime.results[std::format("{}-{}", result_id, workunit_id)];
    timeline.result_id = result_id;
    timeline.workunit_id = workunit_id;
    timeline.events.push_back({event_type, timestamp - timeline.last_tmp});
    timeline.last_tmp = timestamp;
}

std::string string_from_period(PassedPeriod period)
{
    switch (period)
    {
    case PassedPeriod::CreatedAt:
        return "CreatedAt";
    case PassedPeriod::ChosenToSend:
        return "ChosenToSend";
    case PassedPeriod::MovedToClient:
        return "MovedToClient";
    case PassedPeriod::MovedToRunable:
        return "MovedToRunable";
    case PassedPeriod::Ran:
        return "Ran";
    case PassedPeriod::Stopped:
        return "Stopped";
    case PassedPeriod::Wait:
        return "Wait";
    case PassedPeriod::BeforeMissedDeadline:
        return "BeforeMissedDeadline";
    case PassedPeriod::BeforeSendBack:
        return "BeforeSendBack";
    case PassedPeriod::BeforeValidation:
        return "BeforeValidation";
    default:
        return "Not defined";
    }
}

void form_csv_time_ration(const std::string &log_path, LifetimeT &all_client_liftetim_, const std::string & = "")
{
    std::ofstream in_csv(log_path.substr(0, log_path.size() - std::string("avalab").size()) + std::string("time_devision.csv"));
    std::set<std::string> all_event_names;

    for (auto [_, result_timeline] : all_result_liftetime.results)
    {
        for (auto &[even_name, _] : result_timeline.events)
        {
            all_event_names.insert(string_from_period(even_name));
        }
    }
    in_csv << "redsult_id,workunit_id";

    for (auto event_name : all_event_names)
    {
        in_csv << ',' << event_name;
    }
    in_csv << '\n';

    for (auto [_, result_timeline] : all_result_liftetime.results)
    {
        std::map<std::string, double> aggregate_event;
        for (auto &[even_name, timespan] : result_timeline.events)
        {
            aggregate_event[string_from_period(even_name)] += timespan;
        }

        in_csv << result_timeline.result_id << ',' << result_timeline.workunit_id;

        for (auto event_name : all_event_names)
        {
            in_csv << ',' << aggregate_event[event_name];
        }
        in_csv << '\n';
    }
    in_csv << std::endl;
    in_csv.close();
}

void post_stats_work()
{
    // std::string log_path = std::format("/home/ksenia/vsc/combos/traces/{}_{}_avalab", __DATE__, __TIME__);
    std::string log_path = std::format("/home/ksenia/vsc/mirror_combos/traces/{}_{}_avalab", "cur_date", "cur_time");
    form_csv_time_ration(log_path, all_result_liftetime);
}

/**
 * @attention in overall, I understand it, but if I'm not mistaken, this code is buggy
 * I'd like to correct it with documentation from boinc
 */

#include "data_client.hpp"
#include <iostream>
#include <simgrid/s4u.hpp>
#include <math.h>
#include <inttypes.h>

#include "components/types.hpp"
#include "components/shared.hpp"
#include "rand.h"

/*
 *	Disk access data clients simulation
 */
void disk_access(int64_t size)
{
    // Calculate sleep time
    double sleep = std::min((double)maxtt - sg4::Engine::get_clock() - PRECISION, (double)size / 80000000);
    if (sleep < 0)
        sleep = 0;

    // Sleep
    sg4::this_actor::sleep_for(sleep);
}

static sg4::MutexPtr _dclient_mutex = sg4::Mutex::create(); // Data client mutex

/*
 *	Data client ask for input files
 */
int data_client_ask_for_files(ask_for_files_t params)
{

    std::string server_name;
    // msg_error_t error;
    simgrid::s4u::CommPtr comm = NULL; // Asynchronous communication
    // double backoff = 300;

    // Request to data client server
    dcsmessage_t dcsrequest = NULL; // Message to data client server

    // Reply from data client server
    dcmessage_t dcreply = NULL;

    // Request to data server
    dsmessage_t dsinput_file_request = NULL;

    // Reply from data server
    int *dsinput_file_reply_task = nullptr;

    // Reply to data client server
    dcsmessage_t dcsreply = NULL;

    // group_t group_info = NULL;			// Group information
    dclient_t dclient_info = NULL;         // Data client information
    double storage = 0, max_storage = 0;   // File storage in MB
    char project_number, project_priority; // Project number and priority
    int i;

    // params = MSG_process_get_data(MSG_process_self());
    project_number = params->project_number;
    project_priority = params->project_priority;
    // todo: doesn't we need it? So?
    // group_info = params->group_info;
    dclient_info = params->dclient_info;
    sg4::Mailbox *mailbox = sg4::Mailbox::by_name(params->mailbox);

    max_storage = storage = (project_priority / dclient_info->sum_priority) * dclient_info->total_storage * KB * KB;

    ProjectDatabaseValue &project = SharedDatabase::_pdatabase[(int)project_number]; // Database

    // Reduce input file storage if output files are uploaded to data clients
    if (project.output_file_storage == 1)
    {
        max_storage /= 2.0;
        storage = max_storage;
    }

    project.dcmutex->lock();
    // ksenia: not understand logic behind this initalization
    for (i = 0; i < project.ndata_clients; i++)
    {
        if (project.data_clients[i].empty())
        {
            project.data_clients[i] = dclient_info->server_name;
            break;
        }
    }
    project.dcmutex->unlock();

    while (1)
    {

        dclient_info->ask_for_files_mutex->lock();
        if (dclient_info->finish)
        {
            dclient_info->ask_for_files_mutex->unlock();
            break;
        }
        dclient_info->ask_for_files_mutex->unlock();

        // Delete local files when there are completed workunits
        while (storage < max_storage)
        {
            project.dcmutex->lock();
            if (project.ncurrent_deleted_workunits >= project.averagewpif)
            {
                project.ncurrent_deleted_workunits -= project.averagewpif;
                storage += project.input_file_size;
            }
            else
            {
                project.dcmutex->unlock();
                break;
            }
            project.dcmutex->unlock();
        }

        if (storage >= 0)
        {
            fprintf(log_fd, "%g\n", sg4::Engine::get_clock());

            // backoff = 300;

            // ASK FOR WORKUNITS -> DATA CLIENT SERVER
            dcsrequest = new s_dcsmessage_t();
            dcsrequest->type = REQUEST;
            dcsrequest->content = new s_dcsrequest_t();
            dcsrequest->datatype = dcsmessage_content::SDcsrequestT;
            ((dcsrequest_t)dcsrequest->content)->answer_mailbox = mailbox->get_name();

            auto where = project.data_client_servers[uniform_int(0, project.ndata_client_servers - 1)];

            sg4::Mailbox::by_name(where)->put(dcsrequest, KB);

            dcreply = mailbox->get<dcmessage>();

            if (dcreply->nworkunits > 0)
            {
                // ASK FOR INPUT FILES -> DATA SERVERS
                for (auto &[key, workunit] : dcreply->workunits)
                {
                    if (workunit->status != IN_PROGRESS)
                        continue;

                    // Download input files (or generate them locally)
                    if (uniform_int(0, 99) < (int)project.ifgl_percentage)
                    {
                        // Download only if the workunit was not downloaded previously
                        if (uniform_int(0, 99) < (int)project.ifcd_percentage)
                        {
                            for (i = 0; i < workunit->ninput_files; i++)
                            {

                                /**
                                 * ksenia: this piece of code makes me sick.
                                 * result->input_files == workunit.input_files
                                 * workunit.input_files[i] is empty when it is stored on data_client (client side)
                                 * and not empty when on data_serter (server side)
                                 * and ninput_files isn't files as I get it - but replicas.
                                 *
                                 *
                                 * so in the code below we skip if it's sotred on client side, and only do work - if on server
                                 * but in "BORRAR (esta mal)" [eng - Delete (not true)] we are working with client side
                                 *
                                 * and even the first [continue] is strange here because if above [ninput_files] are replicas
                                 * we need only one location
                                 *
                                 * I'd like to check that my understanding is correct before rewritting
                                 */

                                if (workunit->input_files[i].empty())
                                    continue;

                                server_name = workunit->input_files[i];

                                // BORRAR (esta mal, no generico)
                                if (i < project.dcreplication)
                                {
                                    int server_number = atoi(server_name.c_str() + 2) - g_total_number_ordinary_clients;
                                    // printf("resto: %d, server_name: %s, server_number: %d\n", g_total_number_ordinary_clients, server_name.c_str(), server_number);
                                    // printf("%d\n", SharedDatabase::_dclient_info[server_number].working);
                                    // ksenia: seems like checking that our server is working? and if not then lost workunit
                                    if (SharedDatabase::_dclient_info[server_number].working.load() == 0)
                                        continue;
                                }

                                dsinput_file_request = new s_dsmessage_t();
                                dsinput_file_request->type = REQUEST;
                                dsinput_file_request->answer_mailbox = mailbox->get_name();

                                server_name = workunit->input_files[i];

                                sg4::Mailbox::by_name(server_name)->put(dsinput_file_request, KB);

                                dsinput_file_reply_task = mailbox->get<int>();

                                // Timeout reached -> exponential backoff 2^N
                                /*if(error == MSG_TIMEOUT){
                                    backoff*=2;
                                    //free(dsinput_file_request);
                                    continue;
                                }*/

                                // Log request
                                project.rfiles_mutex->lock();
                                project.dcrfiles[i]++;
                                project.rfiles_mutex->unlock();

                                storage -= project.input_file_size;
                                delete dsinput_file_reply_task;
                                dsinput_file_reply_task = NULL;
                                break;
                            }
                        }
                    }

                    // ksenia: i see not reason for this break. Previous there was only one workunit
                    // break;
                }

                // CONFIRMATION MESSAGE TO DATA CLIENT SERVER
                dcsreply = new s_dcsmessage_t();
                dcsreply->type = REPLY;
                dcsreply->content = new s_dcsreply_t();
                dcsreply->datatype = dcsmessage_content::SDcsreplyT;
                ((dcsreply_t)dcsreply->content)->dclient_name = dclient_info->server_name;
                ((dcsreply_t)dcsreply->content)->workunits = dcreply->workunits;

                sg4::Mailbox::by_name(dcreply->answer_mailbox)->put(dcsreply, REPLY_SIZE);
            }
            else
            {
                // Sleep if there are no workunits
                sg4::this_actor::sleep_for(1800);
            }

            delete (dcreply);
            dcsrequest = NULL;
            dcreply = NULL;
        }
        // Sleep if
        if (sg4::Engine::get_clock() >= maxwt || storage <= 0)
            sg4::this_actor::sleep_for(60);
    }

    // Finish data client servers execution
    _dclient_mutex->lock();
    project.nfinished_dclients++;
    if (project.nfinished_dclients == project.ndata_clients)
    {
        for (i = 0; i < project.ndata_client_servers; i++)
        {
            dcsrequest = new s_dcsmessage_t();
            dcsrequest->type = TERMINATION;

            sg4::Mailbox::by_name(project.data_client_servers[i])->put(dcsrequest, REQUEST_SIZE);
        }
    }
    _dclient_mutex->unlock();
    // mailbox.~Mailbox();
    return 0;
}

/*
 *	Data client requests function
 */
int data_client_requests(int argc, char *argv[])
{
    dsmessage_t msg = NULL;                      // Client message
    group_t group_info = NULL;                   // Group information
    dclient_t dclient_info = NULL;               // Data client information
    ask_for_files_t ask_for_files_params = NULL; // Ask for files params
    int32_t data_client_number, group_number;    // Data client number, group number
    int count = 0;                               // Index, termination count

    // Availability params
    double time = 0, random;

    // Check number of arguments
    if (argc != 3)
    {
        printf("Invalid number of parameters in data_client_requests\n");
        return 0;
    }

    // Init data client
    group_number = (int32_t)atoi(argv[1]);       // Group number
    data_client_number = (int32_t)atoi(argv[2]); // Data client number

    group_info = &SharedDatabase::_group_info[group_number];             // Group info
    dclient_info = &SharedDatabase::_dclient_info[data_client_number];   // Data client info
    dclient_info->server_name = sg4::this_actor::get_host()->get_name(); // Server name
    dclient_info->navailable = 0;

    // Wait until group info is ready
    {
        std::unique_lock lock(*group_info->mutex);
        while (group_info->on == 0)
            group_info->cond->wait(lock);
    }

    dclient_info->working.store(uniform_int(1, 2));
    dclient_info->total_storage = (int32_t)ran_distri(group_info->db_distri, group_info->da_param, group_info->db_param);

    // Create ask for files processes (1 per attached project)
    dclient_info->nprojects = atoi(group_info->proj_args[0]);
    dclient_info->sum_priority = 0;
    for (int i = 0; i < dclient_info->nprojects; i++)
        dclient_info->sum_priority += (double)atof(group_info->proj_args[i * 3 + 3]);
    for (int i = 0; i < dclient_info->nprojects; i++)
    {
        ask_for_files_params = new s_ask_for_files_t();
        ask_for_files_params->project_number = (char)atoi(group_info->proj_args[i * 3 + 2]);
        ask_for_files_params->project_priority = (char)atoi(group_info->proj_args[i * 3 + 3]);
        ask_for_files_params->group_info = group_info;
        ask_for_files_params->dclient_info = dclient_info;
        ask_for_files_params->mailbox = bprintf("%s%d", dclient_info->server_name.c_str(), ask_for_files_params->project_number);

        sg4::Actor::create("ask_for_files_thread", sg4::this_actor::get_host(), data_client_ask_for_files, ask_for_files_params);
    }

    sg4::Mailbox *mailbox = sg4::Mailbox::by_name(dclient_info->server_name);

    while (1)
    {

        // Available
        if (dclient_info->working.load() == 2)
        {
            dclient_info->working.store(1);
            random = (ran_distri(group_info->av_distri, group_info->aa_param, group_info->ab_param) * 3600.0);
            if (ceil(random + sg4::Engine::get_clock() >= maxtt))
                random = (double)std::max(maxtt - sg4::Engine::get_clock(), 0.0);
            time = sg4::Engine::get_clock() + random;
        }

        // Non available
        if (dclient_info->working.load() == 1 && ceil(sg4::Engine::get_clock()) >= time)
        {
            random = (ran_distri(group_info->nv_distri, group_info->na_param, group_info->nb_param) * 3600.0);
            if (ceil(random + sg4::Engine::get_clock() >= maxtt))
                random = (double)std::max(maxtt - sg4::Engine::get_clock(), 0.0);
            if (random > 0)
                dclient_info->working.store(0);
            dclient_info->navailable += random;
            sg4::this_actor::sleep_for(random);
            dclient_info->working.store(2);
        }

        // Receive message
        msg = mailbox->get<dsmessage>();

        // Termination message
        if (msg->type == TERMINATION)
        {
            delete (msg);
            count++;
            if (count == dclient_info->nprojects)
                break;
            msg = NULL;
            continue;
        }

        // Insert request into queue
        dclient_info->mutex->lock();
        dclient_info->Nqueue++;
        dclient_info->client_requests.push(msg);

        // If queue is not empty, wake up dispatcher process
        if (dclient_info->Nqueue > 0)
            dclient_info->cond->notify_all();
        dclient_info->mutex->unlock();

        // Free
        msg = NULL;
    }

    // Terminate dispatcher execution
    dclient_info->mutex->lock();
    dclient_info->EmptyQueue = 1;
    dclient_info->cond->notify_all();
    dclient_info->mutex->unlock();

    return 0;
}

/*
 *	Data client dispatcher function
 */
int data_client_dispatcher(int argc, char *argv[])
{
    simgrid::s4u::CommPtr comm = NULL; // Asynchronous comm
    dsmessage_t msg = NULL;            // Client message
    dclient_t dclient_info = NULL;     // Data client information
    int32_t data_client_number;        // Data client number
    double t0, t1;                     // Time

    // Check number of arguments
    if (argc != 3)
    {
        printf("Invalid number of parameters in data_client_dispatcher\n");
        return 0;
    }

    // Init data client
    data_client_number = (int32_t)atoi(argv[2]); // Data client number

    dclient_info = &SharedDatabase::_dclient_info[data_client_number]; // Data client info

    std::vector<sg4::CommPtr> _dscomm;

    while (1)
    {
        std::unique_lock lock(*dclient_info->mutex);

        // Wait until queue is not empty
        while ((dclient_info->Nqueue == 0) && (dclient_info->EmptyQueue == 0))
        {
            dclient_info->cond->wait(lock);
        }

        // Exit the loop when requests function indicates it
        if ((dclient_info->EmptyQueue == 1) && (dclient_info->Nqueue == 0))
        {
            break;
        }

        // Pop client message
        msg = dclient_info->client_requests.front();
        dclient_info->client_requests.pop();
        dclient_info->Nqueue--;
        lock.unlock();

        t0 = sg4::Engine::get_clock();

        // Simulate server computation
        compute_server(20);

        ProjectDatabaseValue &project = SharedDatabase::_pdatabase[(int)msg->proj_number];

        // Reply with output file
        if (msg->type == REPLY)
        {
            disk_access(project.output_file_size);
        }
        // Input file request
        else if (msg->type == REQUEST)
        {
            // Read tasks from disk
            disk_access(project.input_file_size);

            // Create the message

            // Answer the client
            comm = sg4::Mailbox::by_name(msg->answer_mailbox)->put_async(new int(2), project.input_file_size);

            // Store the asynchronous communication created in the dictionary
            delete_completed_communications(_dscomm);
            _dscomm.push_back(comm);
        }

        delete (msg);
        msg = NULL;

        // Iteration end time
        t1 = sg4::Engine::get_clock();

        // Accumulate total time server is busy
        if (t0 < maxtt && t0 >= maxwt)
            dclient_info->time_busy += (t1 - t0);
    }

    dclient_info->ask_for_files_mutex->lock();
    dclient_info->finish = 1;
    dclient_info->ask_for_files_mutex->unlock();

    return 0;
}

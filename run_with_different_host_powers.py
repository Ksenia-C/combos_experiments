# write parameters.xml file with two projects, gather output of simulator and get tasks executed successfully and make plot 

proj1 = """
<sproject>
		<snumber>0</snumber>				<!-- Project number -->
		<name>ATLAS@home</name>				<!-- Project name -->
		<nscheduling_servers>1</nscheduling_servers>	<!-- Number of scheduling servers -->
		<ndata_servers>1</ndata_servers>		<!-- Number of data servers of the project -->
		<ndata_client_servers>1</ndata_client_servers>  <!-- Number of data client servers -->
		<server_pw>12000000000f</server_pw>		<!-- Server power in FLOPS -->
		<disk_bw>167772160</disk_bw>			<!-- Disk speed in bytes/sec -->
		<ifgl_percentage>100</ifgl_percentage>           <!-- Percentage of input files generated locally -->
        <ifcd_percentage>100</ifcd_percentage>           <!-- Percentage of times clients must download new input files (they can't use old input files) -->
		<averagewpif>1</averagewpif>			<!-- Average number of workunits that share the same input files -->
		<input_file_size>57257984</input_file_size>	<!-- Input file size in bytes -->
		<task_fpops>6040000000000</task_fpops>		<!-- Task duration in flops -->
		<output_file_size>104857600</output_file_size>	<!-- Answer size in bytes -->
		<min_quorum>2</min_quorum>			<!-- Number of times a workunit must be received in order to validate it-->
		<target_nresults>2</target_nresults>		<!-- Number of results to create initially -->
		<max_error_results>2</max_error_results>	<!-- Max number of erroneous results -->
		<max_total_results>4</max_total_results>	<!-- Max number of total results -->
		<max_success_results>3</max_success_results>	<!-- Max number of success results -->
		<delay_bound>1000000000</delay_bound>		<!-- Task deadline -->	
		<success_percentage>95</success_percentage>	<!-- Percentage of success results -->
		<canonical_percentage>95</canonical_percentage> <!-- Percentage of success results that make up a consensus -->
		<output_file_storage>0</output_file_storage>	<!-- Output file storage [0 -> data servers, 1 -> data clients] -->
		<dsreplication>1</dsreplication>		<!-- Files replication in data servers -->
		<dcreplication>3</dcreplication>		<!-- Files replication in data clients -->
	<sproject/>
"""

prolog = """
<simulation_time>100</simulation_time>				<!-- Simulation time in hours  -->
<warm_up_time>20</warm_up_time>				<!-- Warm up time in hours -->

<!-- Server side -->
<server_side>
	<n_projects>1</n_projects>				<!-- Number of projects -->
"""
middlepoint = """
	
<server_side/>

<client_side>
	<n_groups>1</n_groups>					<!-- Number of groups -->
	<group>
		<n_clients>8910</n_clients>			<!-- Number of clients of the group -->
		<ndata_clients>100</ndata_clients>		<!-- Number of data clients of the group -->
		<connection_interval>60</connection_interval>	<!-- Connection interval -->	
		<scheduling_interval>3600</scheduling_interval>	<!-- Scheduling interval -->
		<gbw>50Mbps</gbw>				<!-- Cluster link bandwidth in bps -->
		<glatency>7.3ms</glatency>			<!-- Cluster link latency -->
	
"""

epilog = """
	<db_distri>5</db_distri>			<!-- Disk speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<da_param>0.0001</da_param>			<!-- A -->
		<db_param>-1</db_param>				<!-- B -->

		<av_distri>0</av_distri>			<!-- Availability fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<aa_param>0.393</aa_param>			<!-- A -->
		<ab_param>2.964</ab_param>			<!-- B -->

		<nv_distri>2</nv_distri>			<!-- Non-availability fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<na_param>2.444</na_param>			<!-- A -->
		<nb_param>-0.586</nb_param>			<!-- B -->

		<att_projs>1</att_projs>			<!-- Number of projects attached -->
		<gproject>
			<pnumber>0</pnumber>			<!-- Project number -->
			<priority>1</priority>			<!-- Project priority -->
			<lsbw>10Gbps</lsbw>			<!-- Link bandwidth (between group and scheduling servers) -->
			<lslatency>50us</lslatency>		<!-- Link latency (between group and scheduling servers) -->
			<ldbw>10Gbps</ldbw>			<!-- Link bandwidth (between group and data servers) -->
			<ldlatency>50us</latency>		<!-- Link latency (between group and data servers) -->	
		<gproject/>	
	<group/>
<client_side/>
"""

different_power_hosts_with_files = {"seti":
'''
		<traces_file>/Traces/seti/unsorted</traces_file>	<!-- Host power traces file-->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.0534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
''', "einstein":
'''
		<traces_file>/Traces/einstein/unsorted</traces_file>	<!-- Host power traces file-->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.0534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
''', "lhc":
'''
		<traces_file>/Traces/lhc/unsorted</traces_file>	<!-- Host power traces file-->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.0534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
'''

}

different_power_hosts = {"lam=0.2534":
'''
		<!-- <traces_file>/Traces/lhc/unsorted</traces_file>	Host power traces file -->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.2534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
''', "lam=0.1534":
'''
		<!-- <traces_file>/Traces/lhc/unsorted</traces_file>	Host power traces file -->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.1534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
''', "lam=0.0534":
'''
		<!-- <traces_file>/Traces/lhc/unsorted</traces_file>	Host power traces file -->

		<max_speed>117.71</max_speed>			<!-- Maximum host speed in GFlops -->
		<min_speed>0.07</min_speed>			<!-- Minumum host speed in GFlops -->
		<pv_distri>5</pv_distri>			<!-- Speed fit distribution [ran_weibull, ran_gamma, ran_lognormal, normal, hyperx, exponential] -->
		<pa_param>0.0534</pa_param>			<!-- A -->
		<pb_param>-1</pb_param>				<!-- B -->
'''

}


import subprocess
def run_in_schell(cmd: str):
    command = cmd
    try:
        result = subprocess.run(command, shell=True, check=True,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        raise e


key_to_look_at = ['FLOPS average:']
SUM_PRIO = 10
with open("result_for_diff_power.csv", "w") as fout:
    print("expr,FLOPS", file=fout)

    for expr_num in different_power_hosts.keys():
        # form parameters file and all linked
        with open("parameters.xml", "w") as wfile:
            print(prolog, file=wfile)
            print(proj1, file=wfile)
            print(middlepoint, file=wfile)
            print(different_power_hosts[expr_num], file=wfile)
            print(epilog, file=wfile)
        run_in_schell("./generator")


        # run SUM_PRIO times and save in file.
        # in ./generator they shuffle file with host power, so it
        # must be included here, no above.
        for i in range(SUM_PRIO):
            print(i)
            get_macro_stat = run_in_schell("./execute")
            
            get_stat = None
            
            for line in get_macro_stat.split('\n'):
                if key_to_look_at[0] not in line:
                    continue 
                #     Results valid:                910 (80.7%)
                get_stat = line.strip().split(key_to_look_at[0])[1].strip().split(' ')[0].replace(',', '')
            print(f"{expr_num},{get_stat}", file=fout)



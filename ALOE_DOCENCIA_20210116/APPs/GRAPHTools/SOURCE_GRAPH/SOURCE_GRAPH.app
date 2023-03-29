object {
	obj_name=data_source
	exe_name=data_source
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH
	}
}
#######################################################################DATASOURCE
#######################################################################GRAPH
object {
	obj_name=GRAPH				
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=data_source
	}
}


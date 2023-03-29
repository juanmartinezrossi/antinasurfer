object {
	obj_name=data_source_1024
	exe_name=data_source
	outputs {
		name=output_0
		remote_itf=input
		remote_obj=DAC_JACK1
	}
}
#######################################################################DATASOURCE
#######################################################################DAC_JACK
object {
	obj_name=DAC_JACK1
	exe_name=DAC_JACK1
	inputs {
		name=input
		remote_itf=output_0
		remote_obj=data_source_1024
	}
}

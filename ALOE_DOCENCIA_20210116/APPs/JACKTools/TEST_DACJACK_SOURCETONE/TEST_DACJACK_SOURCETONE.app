object {
	obj_name=data_source
	exe_name=data_source
#	kopts=4
	outputs {
		name=output_0
		remote_itf=input
		remote_obj=DAC_JACK0
#		delay=2
#		kbpts=0.001
	}
}
#######################################################################DATASOURCE
#######################################################################DAC_JACK
object {
	obj_name=DAC_JACK0
	exe_name=DAC_JACK0
#	force_pe=0
#	kopts=15
	inputs {
		name=input
		remote_itf=output_0
		remote_obj=data_source
	}
}

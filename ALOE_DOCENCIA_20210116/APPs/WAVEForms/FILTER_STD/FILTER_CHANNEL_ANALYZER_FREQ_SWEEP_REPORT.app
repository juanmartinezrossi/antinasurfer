object {
	obj_name=FILTER_REPORT
	exe_name=CPLX_FILTER_REPORT
#	force_pe=0
	kopts=15
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=FREQ_SWEEP
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=FREQ_SWEEP
	}
}
#########################################################################DAC_JACK0
#########################################################################CHANNEL_ANALYZER
object {
	obj_name=FREQ_SWEEP				
	exe_name=CHANNEL_ANALYZER			
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=FILTER_REPORT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=FILTER_REPORT
	}
	outputs {
		name=output_1
		remote_itf=input_0
		remote_obj=GRAPH
	}
}
#########################################################################CHANNEL_ANALYZER
#########################################################################GRAPH
object {
	obj_name=GRAPH				
	exe_name=GRAPH			
	inputs {
		name=input_0
		remote_itf=output_1
		remote_obj=FREQ_SWEEP
	}
}

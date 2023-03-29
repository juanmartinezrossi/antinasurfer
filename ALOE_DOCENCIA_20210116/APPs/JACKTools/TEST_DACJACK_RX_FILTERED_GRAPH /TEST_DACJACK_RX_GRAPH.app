object {
	obj_name=DAC_JACK0
	exe_name=DAC_JACK0
	kopts=15
	outputs {
		name=output
		remote_itf=input_0
		remote_obj=GRAPH
	}
}
#####################################################DAC_JACK0
#####################################################GRAPH
object {
	obj_name=GRAPH				
	exe_name=GRAPH			
	inputs {
		name=input_0
		remote_itf=output
		remote_obj=DAC_JACK0
	}
}

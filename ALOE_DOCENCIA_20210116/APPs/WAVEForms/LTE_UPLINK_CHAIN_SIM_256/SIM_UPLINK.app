########################################
object {
	obj_name=DATASOURCESINK
	exe_name=DATASOURCESINK_REPORTP21
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UNCRC
	}
	inputs {
		name=input_1
		remote_itf=output_1
		remote_obj=UPLINK_MAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CRC
	}
	outputs {
		name=output_1
		remote_itf=input_0
		remote_obj=GRAPH_BER
	}
}
########################################
########################################
object {
	obj_name=CRC
	exe_name=CRC
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DATASOURCESINK
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LTETURBOTX
	}
}
########################################
########################################
object {
	obj_name=LTETURBOTX
	exe_name=LTEturboCOD2
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CRC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=SCRAMBLING
	}
}
########################################
########################################SCRAMBLING
object {
	obj_name=SCRAMBLING
	exe_name=SCRAMBLING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LTETURBOTX
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=MOD_16QAM
	}
}
########################################SCRAMBLING
########################################
object {
	obj_name=MOD_16QAM
	exe_name=MOD_QAM
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=SCRAMBLING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DFT
	}
}
########################################MOD_QAM
########################################DFT
object {
	obj_name=DFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=MOD_16QAM
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_MAPPING
	}
}
########################################DFT
########################################UPLINK_MAPPING
object {
	obj_name=UPLINK_MAPPING				
	exe_name=UPLINK_MAPPING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_MAPPING
	}
	outputs {
		name=output_1
		remote_itf=input_1
		remote_obj=DATASOURCESINK
	}
}
####################################################################MAPPINGIV
####################################################################DEC17_GRAPH_MAPPINGIV
object {
	obj_name=GRAPH_MAPPING
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_MAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=IFFT
	}
}
####################################################################DEC17_GRAPH_MAPPINGIV
########################################IFFT
object {
	obj_name=IFFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_MAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_IFFT
	}
}
########################################IFFT
########################################GRAPH_IFFT
object {
	obj_name=GRAPH_IFFT
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DUC17
	}
}
########################################GRAPH_IFFT
#####################################################DUC17
object {
	obj_name=DUC17
	exe_name=DUC17
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_IFFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_DUC
	}
}
####################################################################DUC
####################################################################GRAPH_DUC
object {
	obj_name=GRAPH_DUC			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DUC17
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CIRC_BUFFER_TX	
	}
}
####################################################################GRAPH_DUC
####################################################################CIR_BUFFER_TX
object {
	obj_name=CIRC_BUFFER_TX				
	exe_name=CIRC_BUFFER
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_DUC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DISTOR_CHANNEL
	}
}
####################################################################CIR_BUFFER_TX
#####################################################IFFT
object {
	obj_name=DISTOR_CHANNEL
	exe_name=CPLX_FILTER
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CIRC_BUFFER_TX
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CHAN_NOISE
	}
}
####################################################IFFT
####################################################################CHANNEL_NOISE
object {
	obj_name=CHAN_NOISE			
	exe_name=channel_noise
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DISTOR_CHANNEL
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_CHANNEL
	}
}
####################################################################CHANNEL_NOISE

####################################################################GRAPH_CHANNEL
object {
	obj_name=GRAPH_CHANNEL
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CHAN_NOISE	
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=CIRC_BUFFER_RX
	}
}
####################################################################GRAPH_CHANNEL
####################################################################CIR_BUFFER_RX
object {
	obj_name=CIRC_BUFFER_RX				
	exe_name=CIRC_BUFFER
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_CHANNEL
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DDC17
	}
}
####################################################################CIR_BUFFER_RX
#######################################################################DDC17
object {
	obj_name=DDC17
	exe_name=DDC17
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=CIRC_BUFFER_RX
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_DDC
	}
}
####################################################################DDC17
####################################################################GRAPH_DDC
object {
	obj_name=GRAPH_DDC			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DDC17
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_SYNCHRO	
	}
}
####################################################################GRAPH_DDC
######################################################################UPLINK_SYNCHRO
object {
	obj_name=UPLINK_SYNCHRO
	exe_name=UPLINK_SYNCHRO
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_DDC
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=FFT
	}
	outputs {
		name=output_1
		remote_itf=input_0
		remote_obj=GRAPH_OUT1
	}
	outputs {
		name=output_2
		remote_itf=input_0
		remote_obj=GRAPH_OUT2
	}
}
#####################################################################UPLINK_SYNCHRO
########################################FFT
object {
	obj_name=FFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_SYNCHRO
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_DEMAPPING
	}
}
########################################FFT
####################################################################DEMAPPING
object {
	obj_name=UPLINK_DEMAPPING				
	exe_name=UPLINK_DEMAPPING
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=FFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_DEMAPPING
	}

}
####################################################################DEMAPPING
####################################################################GRAPH_FFT
object {
	obj_name=GRAPH_DEMAPPING			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_DEMAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UPLINK_EQUALIZER	
	}
}
####################################################################GRAPH_FFT
####################################################################EQUALIZER
object {
	obj_name=UPLINK_EQUALIZER				
	exe_name=UPLINK_EQUALIZER
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_DEMAPPING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_EQUALIZER
	}
	outputs {
		name=output_1
		remote_itf=input_0
		remote_obj=GRAPH_CAPTUREDMRS
	}
	outputs {
		name=output_2
		remote_itf=input_0
		remote_obj=GRAPH_COMPUTEDMRS
	}
}
####################################################################EQUALIZER
###################################################################GRAPH_CAPTUREDMRS
object {
	obj_name=GRAPH_CAPTUREDMRS			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_1
		remote_obj=UPLINK_EQUALIZER
	}
}
###################################################################GRAPH_CAPTUREDMRS
###################################################################GRAPH_CAPTUREDMRS
object {
	obj_name=GRAPH_COMPUTEDMRS			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_2
		remote_obj=UPLINK_EQUALIZER
	}
}
###################################################################GRAPH_CAPTUREDMRS
####################################################################GRAPH_FFT
object {
	obj_name=GRAPH_EQUALIZER			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=UPLINK_EQUALIZER
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=IDFT	
	}
}

########################################IDFT
object {
	obj_name=IDFT
	exe_name=FFT_IFFT
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_EQUALIZER
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=GRAPH_IDFT
	}
}
########################################DFT
####################################################################GRAPH_FFT
object {
	obj_name=GRAPH_IDFT			
	exe_name=GRAPH
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=IDFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DEMOD_16QAM	
	}
}
####################################################################GRAPH_FFT
####################################################################DEMOD16QAM
object {
	obj_name=DEMOD_16QAM
	exe_name=MOD_QAM
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=GRAPH_IDFT
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DESCRAMBLING
	}
}
####################################################################DEMOD16QAM
################################################DESCRAMBLING
object {
	obj_name=DESCRAMBLING
	exe_name=SCRAMBLING
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DEMOD_16QAM
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=LTETURBORXFLOATS
	}
}
################################################DESCRAMBLING
#######################################################################TURBODECODER
object {
	obj_name=LTETURBORXFLOATS
	exe_name=LTEturboCOD2
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=DESCRAMBLING
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=UNCRC
	}
}
#######################################################################TURBODECODER
#######################################################################UNCRC
object {
	obj_name=UNCRC
	exe_name=CRC_REPORT
	force_pe=0
	inputs {
		name=input_0
		remote_itf=output_0
		remote_obj=LTETURBORXFLOATS
	}
	outputs {
		name=output_0
		remote_itf=input_0
		remote_obj=DATASOURCESINK 
	}
}
#######################################################################UNCRC

####################################################################GRAPH_OUT1
object {
	obj_name=GRAPH_OUT1
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_1
		remote_obj=UPLINK_SYNCHRO
	}
}
######################################################################GRAPH_OUT1
####################################################################GRAPH_OUT2
object {
	obj_name=GRAPH_OUT2
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_2
		remote_obj=UPLINK_SYNCHRO
	}
}
######################################################################GRAPH_OUT2
####################################################################GRAPH_OUT2
object {
	obj_name=GRAPH_BER
	exe_name=GRAPH		
	inputs {
		name=input_0
		remote_itf=output_1
		remote_obj=DATASOURCESINK
	}
}
######################################################################GRAPH_OUT2
############################END



# To be used in Octave or Matlab
# Read a complex data file with data format: 0.3344+0.5567i
# val defines the complex array
val=dlmread("IFFT_0_OUTPUT.txt", " ",1, 0)
# Get real part array
R=real(val)
# Get the imag part
I=imag(val)
# Scattering plot function
scatter(R,I)


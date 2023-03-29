/*
 * mapper.h
 *
 * Copyright (c) 2009 Vuk Marojevic, UPC <marojevic at tsc.upc.edu>. All rights reserved.
 *
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


/* Constants */
#define Nmax	20	/* Maximum number of processors */
#define Mmax	50	/* Maximum number of application's tasks/processes/SDR functions */

#define Wmax	10	/* Maximum number of processors */

#define infinite 1000000	/* symbol for infinite (mapping cost) */ 
#define inf	100000000	/* symbol for infinite (resource) */ 
#define threshold 0.00000001	/* threshold for comparing two floating point numbers; if their absolute difference less than threshold they are considered equal */

/* Mapping algorithms */
enum algtype{gw, tw};	/* gw- or tw-mapping */

/* Interprocessor communication network */
enum commtype{fd, hd, bus};	/* full-duplex, half-duplex, and bus architectures */

/* Mapping preprocessing (order) selection */
enum ordtype{no_ord, c_ord, b_ord};

/* Resources vectors and matrices */
struct platform_resources {
	int nof_processors; 	/* Number of processors */
	enum commtype arch;	/* specifies the platform's interpreocessor communication network as full-duplex, half-duplex, or shared bus */
	float *C; 		/* The device model contains the processing capacities */
	float **B; 		/* Bandwidth resources */
};

/* Resources vectors and matrices */
/*
- Full-duplex communication architectures (arch = fd): L[i][j] is independent form L[j][i]; I-elements point to different entries in B[]
- Half-duplex communication architectures (arch = hd): L[i][j] = L[j][i] represent the bi-directional bus bandwidth between processor Pj and Pi; I[i][j] = I[j][i] as both point to one entry in B[]
- Bus architectures (arch = bus): L[i][j] = bus_bandwidth for all i, j<>i; all I-elements point to one entry in B[], which contains the bus bandwidth
In case of a mix of dedicated directional links and shared (bidirectional) links, I[][] and B[] must be used instead of L[][] (see PhD dissertation)
*/

/* Waveform vectors and matrices */
struct waveform_resources {
	int nof_tasks; 	/* Number of processing objects (tasks) */
	float *c; 	 	/* The "processing model" contains the processing demands */
	float **b; 	 	/* The "data flow model" specifies the bandwidth requirements */
	int *force;		/* Force the mapping of a task into a processor index */
};

/* Waveform vectors and matrices */
/*
b[] & i[][] (together) model the data flow demands. l[][] is an alternatively data flow model.
*/

struct preprocessing {
	enum ordtype ord;	/**/
};


/* Algorithm selection and configuration structure */
struct mapping_algorithm {
	enum algtype type;	/* Algorithm selector */
	int w; 		/* Window size (for tw-mapping algorithm): 1, 2, 3, 4, 5 (default: w = 1) */
};

/* Cost function selection and configuration structure */
struct cost_function {
	int mhop;		/* Multihop indicator; mhop = 0: single hops (direct interprocessor communication) only; mhop = 1: 2-hops (with one intermediate processor) only if necessary; mhop = 2: single & 2-hops (cost-selective) */
	float q;		/* Cost function parameter; 'q' weigths the cost_comp term and '1-q' the cost_comm term: [0..1] */
};

/* mapping output vector/matrix */
struct mapping_result {
	int *P_m; 		/* Mapping proposal; vector P_m specifies where tasks are mapped: P_m[i] = j, for example, means that task 'i' is mapped to processor 'j' */
};
	

/* API Functions */


/** Run mapper algorithm
 * 
 * Maps a set of waveform objects into a set of processing elements.
 * ...
 * ...
 * @param algorithm configures algorithm
 * @param waveform Sets waveform objects comp/comm demands. 
 * @param resources Sets comp/comm Resources 
 * @return >=0 Mapping cost, -1 if error
 *
 * @caution waveform and resources structure's vectors and matrices dimensions
 * must agree nof_objects/nof_processors values.
 * 
 */
float mapper(struct preprocessing *preproc,
             struct mapping_algorithm *algorithm,
             struct cost_function *costfunc,
             struct platform_resources *platform,
             struct waveform_resources *waveform,
             struct mapping_result *result);
             


/** Declare here your public variables */

/** fill the structure */
struct utils_variables my_vars[] = {
		{"my_variable",		/** variable name */
		STAT_TYPE_INT,		/** type */
		1,					/** size, in number of type elements */
		&my_variable,		/** pointer to the variable */
			READ},			/** Automatic mode:
							READ: obtains the value at the begining of the timeslot
							WRITE: Sets the value at the end of the timeslot
						*/


		/** end */
		{NULL, 0, 0, 0, 0}};


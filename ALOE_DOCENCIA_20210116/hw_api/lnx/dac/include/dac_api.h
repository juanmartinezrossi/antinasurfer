

/** Interface for DAC management.
 * Each dac driver must implement all functions defined in the structure
 * dac_interface.
 *
 * Currently, dac and adc are tuned to the same frequency.
 *
 * All functions return 1 if ok, or <0 if error
 */

struct dac_interface {

	/* this name must match the name selected in platforms.con */
	const char *name;

	int (*readcfg) (struct dac_cfg *cfg);

	/** initialize dac. options may vary between dacs.
	 *
	 *
	 * Sampling is started during this function call.
	 * \cfg Pointer to shared dac_cfg structure. This configuration may be changed on run-time and actions should be adopted by the dac accordingly.
	 * \timeSlot Pointer to write time-slot lenth. This value may be changed on run-time by the DAC.
	 * \sync function that must be called each dac interrupt to synchronize the local processor timer (time slot).
	 */
	int (*init) (struct dac_cfg *cfg, int *timeSlotLength, void (*sync)(void));

	/** close dac */
	void (*close) ();

};

